#ifndef SERVER_H
#define SERVER_H
#include "database_server.h"
#include "utils.h"
typedef enum {
CMD_UNKNOWN, //未知命令
CMD_SHOW, //显示所有设备最新状态
CMD_QUERY, //查询特定设备历史数据
CMD_EXIT //退出程序
} CommandType;
typedef struct {
CommandType type;
char device_id[MAX_DEVICE_ID_LEN]; //设备ID
} Command;
void start_server(void); //启动服务器主函数
void* accept_clients(void* arg); //接受客户端连接的线程函数
#endif