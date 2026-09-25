#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <time.h>
#include "config.h"
#include "utils.h"

// 数据库全局变量
static sqlite3 *db = NULL;

int get_device_count();
// 初始化数据库连接
bool init_query_database() {
    int rc = sqlite3_open("device.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "数据库连接失败: %s\n", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

// 关闭数据库连接
void close_query_database() {
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }
}

// 显示所有设备最新记录（对应 ./query 指令）
void show_all_recent_records() {
    const char *sql = 
        "SELECT id, device_id, timestamp, cpu_usage, memory_usage, temperature, status_code, datetime(timestamp, 'unixepoch', 'localtime') AS received_time "
        "FROM device_status "
        "ORDER BY timestamp DESC LIMIT 10";
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db));
        return;
    }

    int total_count = 0;
    // 统计总记录数
    while (sqlite3_step(stmt) == SQLITE_ROW) total_count++;
    sqlite3_reset(stmt);

    printf("Device Status Database Query Tool\n");
    printf("total=%d devices=%d\n", total_count, get_device_count());
    printf("Recent records:\n");

    int id = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *device_id = (const char *)sqlite3_column_text(stmt, 1);
        time_t timestamp = (time_t)sqlite3_column_int64(stmt, 2);
        double cpu = sqlite3_column_double(stmt, 3);
        double mem = sqlite3_column_double(stmt, 4);
        double temp = sqlite3_column_double(stmt, 5);
        int status = sqlite3_column_int(stmt, 6);
        const char *recv_time = (const char *)sqlite3_column_text(stmt, 7);

        printf("id=%d\n", id++);
        printf("device id=%s\n", device_id);
        printf("timestamp = %ld\n", timestamp);
        printf("cpu_usage=%.1f\n", cpu);
        printf("memory_usage = %.1f\n", mem);
        printf("temperature=%.1f\n", temp);
        printf("status_code=%d\n", status);
        printf("received time=%s\n", recv_time);
        printf("\n");
    }

    sqlite3_finalize(stmt);
}

// 获取设备总数
int get_device_count() {
    const char *sql = "SELECT COUNT(DISTINCT device_id) FROM device_status";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

// 查询指定设备历史数据（对应 ./query DEVICE-XXX 指令）
void query_device_history(const char *device_id) {
    const char *sql = 
        "SELECT id, timestamp, cpu_usage, memory_usage, temperature, status_code, datetime(timestamp, 'unixepoch', 'localtime') AS received_time "
        "FROM device_status "
        "WHERE device_id = ? "
        "ORDER BY timestamp DESC";
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, device_id, -1, SQLITE_STATIC);

    printf("Device Status Database Query Tool\n");
    printf("Query result for device: %s\n", device_id);
    printf("\n");

    int id = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        time_t timestamp = (time_t)sqlite3_column_int64(stmt, 1);
        double cpu = sqlite3_column_double(stmt, 2);
        double mem = sqlite3_column_double(stmt, 3);
        double temp = sqlite3_column_double(stmt, 4);
        int status = sqlite3_column_int(stmt, 5);
        const char *recv_time = (const char *)sqlite3_column_text(stmt, 6);

        printf("id=%d\n", id++);
        printf("device id=%s\n", device_id);
        printf("timestamp = %ld\n", timestamp);
        printf("cpu_usage=%.1f\n", cpu);
        printf("memory_usage = %.1f\n", mem);
        printf("temperature=%.1f\n", temp);
        printf("status_code=%d\n", status);
        printf("received time=%s\n", recv_time);
        printf("\n");
    }

    if (id == 1) {
        printf("设备 %s 无历史数据\n", device_id);
    }

    sqlite3_finalize(stmt);
}

// 查询指定设备统计信息（对应 ./query stats DEVICE-XXX 指令）
void query_device_stats(const char *device_id) {
    const char *sql = 
        "SELECT COUNT(*), AVG(temperature), AVG(cpu_usage), AVG(memory_usage) "
        "FROM device_status "
        "WHERE device_id = ?";
    
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "查询失败: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, device_id, -1, SQLITE_STATIC);

    printf("Device Status Database Query Tool\n");
    printf("device_id = %s\n", device_id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        double avg_temp = sqlite3_column_double(stmt, 1);
        double avg_cpu = sqlite3_column_double(stmt, 2);
        double avg_mem = sqlite3_column_double(stmt, 3);

        printf("count=%d\n", count);
        printf("avg temp=%.2f\n", avg_temp);
        printf("avg_cpu=%.1f\n", avg_cpu);
        printf("avg mem=%.2f\n", avg_mem);
    } else {
        printf("设备 %s 无统计数据\n", device_id);
    }

    sqlite3_finalize(stmt);
}

// 打印使用说明
void print_query_usage(const char *program_name) {
    printf("Usage:\n");
    printf("  %s                - 显示所有设备最新记录\n", program_name);
    printf("  %s <device_id>    - 查询指定设备历史数据（例：%s DEVICE-001）\n", program_name, program_name);
    printf("  %s stats <device_id> - 查询指定设备统计信息（例：%s stats DEVICE-001）\n", program_name, program_name);
}

int main(int argc, char *argv[]) {
    // 初始化数据库
    if (!init_query_database()) {
        exit(EXIT_FAILURE);
    }

    // 解析命令行参数
    if (argc == 1) {
        // 无参数：显示所有记录
        show_all_recent_records();
    } else if (argc == 2) {
        // 一个参数：查询指定设备历史数据
        query_device_history(argv[1]);
    } else if (argc == 3 && strcmp(argv[1], "stats") == 0) {
        // 两个参数且第一个为 stats：查询设备统计信息
        query_device_stats(argv[2]);
    } else {
        // 参数错误：显示使用说明
        print_query_usage(argv[0]);
        close_query_database();
        exit(EXIT_FAILURE);
    }

    // 关闭数据库
    close_query_database();
    return EXIT_SUCCESS;
}