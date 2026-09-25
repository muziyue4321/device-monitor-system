#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>  // 新增：引入errno和EINTR定义
#include "server.h"
#include "database_server.h"

static int server_running = 1; // 服务器运行标志（1:运行中，0:停止）
static int server_sock = -1; // 服务器socket文件描述符

// 信号处理函数
void signal_handler(int sig) {
    printf("\n收到信号 %d，正在关闭服务器...\n", sig);
    server_running = 0; 
    if (server_sock >= 0) {
        close(server_sock);
        server_sock = -1;
    }
}

// 创建并配置服务器socket
int create_server_socket(void) {
    // 创建TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("创建socket失败");
        return -1;
    }
    // 设置socket选项
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("设置socket选项失败");
        close(sock);
        return -1;
    }
    // 配置服务器地址结构
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    // 绑定socket到指定地址和端口
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("绑定地址失败");
        close(sock);
        return -1;
    }
    // 开始监听连接
    if (listen(sock, 10) < 0) {
        perror("监听失败");
        close(sock);
        return -1;
    }
    return sock;
}

// 处理客户端消息
void* client_handler(void* arg) {
    // 获取客户端socket并释放参数内存
    int client_sock = *(int*)arg;
    free(arg);
    char buffer[DATA_BUFFER_SIZE];
    DeviceData data;
    // 循环处理客户端请求，直到连接断开或服务器停止
    while (server_running) {
        // 清空缓冲区并接收数据
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        // 接收失败或连接关闭时退出循环
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("客户端断开连接\n");
            } else if (errno != EINTR) {
                perror("接收数据失败");
            }
            break;
        }

        buffer[bytes_received] = '\0';
        // 解析设备数据字符串：设备ID,时间戳,CPU使用率,内存使用率,温度,状态码
        if (sscanf(buffer, "%19[^,],%ld,%lf,%lf,%lf,%d",
            data.device_id, &data.timestamp, 
            &data.cpu_usage, &data.memory_usage, 
            &data.temperature, &data.status_code) == 6) {
            printf("收到设备数据: %s, CPU: %.1f%%, 内存: %.1f%%\n", 
            data.device_id, data.cpu_usage, data.memory_usage);
            // 将设备数据存入数据库
            if (insert_device_data(&data)) {
                const char* ok_msg = "OK\n";
                if (send(client_sock, ok_msg, strlen(ok_msg), 0) < 0) {
                    perror("发送响应失败");
                }
            } else {
                const char* err_msg = "ERROR: 保存数据失败\n";
                send(client_sock, err_msg, strlen(err_msg), 0);
            }
        } else {
            const char* err_msg = "ERROR: 数据格式错误\n";
            send(client_sock, err_msg, strlen(err_msg), 0);
        }
    }

    close(client_sock);
    return NULL;
}

// 接受客户端连接
void* accept_clients(void* arg) {
    // 获取服务器socket
    server_sock = *(int*)arg;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    printf("开始接受客户端连接...\n");
    // 持续接受新连接，直到服务器停止
    while (server_running) {
        addr_len = sizeof(client_addr);
        // 等待客户端连接
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
            if (errno == EINTR || !server_running) {
                break;
            }
            perror("接受连接失败");
            continue;
        }
        // 打印客户端连接信息
        printf("新的客户端连接: %s:%d\n", 
        inet_ntoa(client_addr.sin_addr), 
        ntohs(client_addr.sin_port));
        // 为客户端动态分配socket描述符存储空间
        int* client_ptr = malloc(sizeof(int));
        if (client_ptr == NULL) {
            perror("内存分配失败");
            close(client_sock);
            continue;
        }
        *client_ptr = client_sock;
        // 创建客户端处理线程
        pthread_t thread;
        if (pthread_create(&thread, NULL, client_handler, client_ptr) != 0) {
            perror("创建客户端线程失败");
            free(client_ptr);
            close(client_sock);
            continue;
        }
        pthread_detach(thread);
    }
    printf("停止接受客户端连接\n");
    return NULL;
}

// 解析命令
Command parse_command(const char* cmd_str) {
    Command cmd = {CMD_UNKNOWN, ""};
    // 移除命令字符串前后的空白字符
    char trimmed[100];
    strncpy(trimmed, cmd_str, sizeof(trimmed));
    trimmed[sizeof(trimmed)-1] = '\0';
    // 去掉末尾换行符
    trimmed[strcspn(trimmed, "\n\r")] = '\0';
    // 命令匹配逻辑
    if (strcmp(trimmed, "show") == 0) {
        cmd.type = CMD_SHOW;
    } else if (strcmp(trimmed, "exit") == 0) {
        cmd.type = CMD_EXIT;
    } else if (strncmp(trimmed, "query ", 6) == 0) {
        cmd.type = CMD_QUERY;
        // 提取设备ID
        sscanf(trimmed + 6, "%19s", cmd.device_id);
    }
    return cmd;
}

// 格式化显示设备数据
void format_display(DeviceData* data, int count, const char* title) {
    printf("\n%s (共 %d 条记录)\n", title, count);
    printf("设备ID 时间戳 CPU%% 内存%% 温度℃ 状态\n");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-12s %-12ld %-8.1f %-9.1f %-10.1f %d\n",
        data[i].device_id, data[i].timestamp,
        data[i].cpu_usage, data[i].memory_usage,
        data[i].temperature, data[i].status_code);
    }
    printf("------------------------------------------------------------\n");
}

// 处理用户命令
void handle_command(Command cmd) {
    int count = 0;
    DeviceData* data = NULL;
    switch (cmd.type) {
        case CMD_SHOW:
            // 查询所有设备的最新状态
            data = get_all_devices_latest_status(&count);
            if (data && count > 0) {
                format_display(data, count, "所有设备最新状态");
                free(data);
            } else {
                printf("没有找到设备数据\n");
            }
            break;
        case CMD_QUERY:
            // 查询特定设备的历史数据
            if (strlen(cmd.device_id) > 0) {
                data = get_device_history(cmd.device_id, &count);
                if (data && count > 0) {
                    format_display(data, count, cmd.device_id);
                    free(data);
                } else {
                    printf("设备 %s 没有历史数据\n", cmd.device_id);
                }
            } else {
                printf("错误：缺少设备ID\n");
            }
            break;
        case CMD_EXIT:
            printf("正在关闭服务器...\n");
            server_running = 0;
            break;
        default:
            printf("可用命令:\n");
            printf(" show - 显示所有设备最新状态\n");
            printf(" query <设备ID> - 查询设备历史数据\n");
            printf(" exit - 退出程序\n");
            break;
    }
}

// 启动服务器主函数
void start_server(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    printf("========== 设备监控服务器 ==========\n");
    // 初始化数据库连接
    if (!init_database()) {
        fprintf(stderr, "错误：数据库初始化失败\n");
        return;
    }
    printf("数据库初始化成功\n");
    // 创建服务器socket
    int sock = create_server_socket();
    if (sock < 0) {
        close_database();
        return;
    }
    server_sock = sock;
    printf("服务器启动成功，监听端口: %d\n", SERVER_PORT);
    printf("等待客户端连接...\n\n");
    // 显示命令帮助信息
    printf("命令说明:\n");
    printf(" show - 显示所有设备最新状态\n");
    printf(" query <设备ID> - 查询设备历史数据\n");
    printf(" exit - 退出程序\n");
    // 创建并启动客户端连接接收线程
    pthread_t accept_thread;
    if (pthread_create(&accept_thread, NULL, accept_clients, &server_sock) != 0) {
        perror("创建接收线程失败");
        close(server_sock);
        close_database();
        return;
    }
    // 主线程
    char command[100];
    while (server_running) {
        // 显示命令提示符
        printf("> ");
        fflush(stdout);
        // 读取用户输入
        if (fgets(command, sizeof(command), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\n检测到文件结束符，退出服务器\n");
                server_running = 0;
            }
            break;
        }
        command[strcspn(command, "\n")] = '\0';
        if (strlen(command) == 0) {
            continue;
        }
        handle_command(parse_command(command));
    }
    printf("\n服务器关闭中...\n");
    if (server_sock >= 0) {
        close(server_sock);
        server_sock = -1;
    }
    void* thread_result;
    pthread_join(accept_thread, &thread_result);
    close_database();
    printf("服务器已正常关闭\n");
}

int main(void) {
    start_server();
    return 0;
}