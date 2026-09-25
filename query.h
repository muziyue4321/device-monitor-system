#ifndef QUERY_H
#define QUERY_H

#include <stdbool.h>
#include "utils.h"

// 初始化数据库连接
bool init_query_database();

// 关闭数据库连接
void close_query_database();

// 显示所有设备最新记录
void show_all_recent_records();

// 查询指定设备历史数据
void query_device_history(const char *device_id);

// 查询指定设备统计信息
void query_device_stats(const char *device_id);

// 打印使用说明
void print_query_usage(const char *program_name);

#endif