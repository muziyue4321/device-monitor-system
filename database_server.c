#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include "database_server.h"
static sqlite3 *db = NULL;
bool init_database(void) {
    // 打开数据库文件，如果没有则创建
    int rc = sqlite3_open("device.db", &db);
    if (rc != SQLITE_OK) {
        perror("open error");
        return false;
    }
// 创建表：如果表不存在则创建 device_status 表
const char *sql = 
"CREATE TABLE IF NOT EXISTS device_status ("
"id INTEGER PRIMARY KEY AUTOINCREMENT,"
"device_id TEXT NOT NULL,"
"timestamp INTEGER NOT NULL,"
"cpu_usage REAL NOT NULL,"
"memory_usage REAL NOT NULL,"
"temperature REAL NOT NULL,"
"status_code INTEGER NOT NULL"
");"
"CREATE INDEX IF NOT EXISTS idx_device_id ON device_status(device_id);"
"CREATE INDEX IF NOT EXISTS idx_timestamp ON device_status(timestamp);";
// 执行SQL语句，创建表和索引
char *err_msg = NULL; 
if (sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) { // 改回 err_msg
perror("create error");
sqlite3_free(err_msg); // 如果执行失败，打印错误信息,释放错误消息
db = NULL;
return false;
}
printf("create successfully\n");
return true;
}
// 关闭数据库
void close_database(void) {
if (db != NULL) {
sqlite3_close(db);//// 关闭数据库连接
db = NULL;
printf("datebase close\n");
}
}
// 插入设备数据
bool insert_device_data(const DeviceData *data) {
sqlite3_stmt *stmt; //SQLite 语句对象的指针,为了在 SQL 语句准备、绑定数据、执行、以及取回结果时进行管理
const char *sql = "INSERT INTO device_status " 
"(device_id, timestamp, cpu_usage,memory_usage, temperature, status_code)"
"VALUES (?, ?, ?, ?, ?, ?)";
//INSERT 语句，用于向 device_status 表插入数据
int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
//预编译 SQL 语句。
if (rc != SQLITE_OK) {
return false;
}
sqlite3_bind_text(stmt, 1, data->device_id, -1, SQLITE_STATIC);// 绑定设备ID
sqlite3_bind_int64(stmt, 2, (long long)data->timestamp);// 绑定时间戳
sqlite3_bind_double(stmt, 3, data->cpu_usage);// 绑定CPU使用率
sqlite3_bind_double(stmt, 4, data->memory_usage);// 绑定内存使用率
sqlite3_bind_double(stmt, 5, data->temperature);// 绑定温度
sqlite3_bind_int(stmt, 6, data->status_code);// 绑定设备状态码
rc = sqlite3_step(stmt);//执行预编译语句
sqlite3_finalize(stmt);//销毁预编译语句
if (rc != SQLITE_DONE) { 
perror("insert false");/// 插入失败，打印错误信息
return false;
}
return true;
}
// 获取所有设备的最新状态 
DeviceData* get_all_devices_latest_status(int* count) {
const char *sql = 
"SELECT device_id, timestamp, cpu_usage, memory_usage, "
"temperature, status_code FROM device_status "
"WHERE (device_id, timestamp) IN ("
" SELECT device_id, MAX(timestamp) "
" FROM device_status "
" GROUP BY device_id"
") ORDER BY device_id";
// 查询所有设备的最新状态，获取每个设备ID的最大时间戳
sqlite3_stmt *stmt;//SQLite 语句对象的指针
if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
perror("find error");
*count = 0;
return NULL;
} // 查询失败，打印错误信息
int total_count = 0;
while (sqlite3_step(stmt) == SQLITE_ROW) {
total_count++;
}// 计算返回结果的记录数,每读到一行数据，记录数加1
// 重置查询
sqlite3_reset(stmt);
// 没有数据，直接返回
if (total_count == 0) {
sqlite3_finalize(stmt);
*count = 0;
return NULL;
}
//如果有记录,分配内存存查询结果
DeviceData *result = (DeviceData*)malloc(total_count * sizeof(DeviceData));
if (result == NULL) {
perror("malloc false");
sqlite3_finalize(stmt);
*count = 0;
return NULL;
}//查询不到就释放资源，并打印错误
// 遍历查询结果，读取每条记录
int i = 0;
while (sqlite3_step(stmt) == SQLITE_ROW && i < total_count) {//迭代访问每一行查询结果
// 读取设备ID
const char *device_id = (const char*)sqlite3_column_text(stmt, 0);
if (device_id != NULL) {
strncpy(result[i].device_id, device_id, MAX_DEVICE_ID_LEN - 1);
result[i].device_id[MAX_DEVICE_ID_LEN - 1] = '\0';
}//复制设备ID，确保在 result[i].device_id 中存储了查询结果中的设备ID
// 时间戳
result[i].timestamp = (time_t)sqlite3_column_int64(stmt, 1);
// CPU使用率
result[i].cpu_usage = sqlite3_column_double(stmt, 2);
// 内存使用率
result[i].memory_usage = sqlite3_column_double(stmt, 3);
// 温度
result[i].temperature = sqlite3_column_double(stmt, 4);
// 状态码
result[i].status_code = sqlite3_column_int(stmt, 5);
i++;//处理下一行数据
}
sqlite3_finalize(stmt);// 释放SQLite指针
*count = total_count;// 返回记录数
return result;// 返回查询结果
}
// 获取设备历史数据 - 独立实现
DeviceData* get_device_history(const char* device_id, int* count) {
sqlite3_stmt *stmt; //SQLite 语句对象的指针
const char *sql = "SELECT device_id, timestamp, cpu_usage, memory_usage, "
"temperature, status_code FROM device_status "
"WHERE device_id = ? ORDER BY timestamp ASC";
// 查询指定设备的所有历史数据
if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
perror("find error");
*count = 0;
return NULL;
}
// 查询失败，打印错误信息
// 绑定设备ID参数
sqlite3_bind_text(stmt, 1, device_id, -1, SQLITE_STATIC);
// 计算记录
int total_count = 0;
while (sqlite3_step(stmt) == SQLITE_ROW) {
total_count++;
}
// 重置查询并重新绑定参数
sqlite3_reset(stmt);
sqlite3_bind_text(stmt, 1, device_id, -1, SQLITE_STATIC);
// 如果没有数据，直接返回
if (total_count == 0) {
sqlite3_finalize(stmt);
*count = 0;
return NULL;
}
//如果有记录,分配内存存查询结果
DeviceData *result = (DeviceData*)malloc(total_count * sizeof(DeviceData));
if (result == NULL) {
perror("malloc false");
sqlite3_finalize(stmt);
*count = 0;
return NULL;
}//查询不到就释放指针，并打印错误信息
// 读取数据
int i = 0;
while (sqlite3_step(stmt) == SQLITE_ROW && i < total_count) {
// 读取设备ID
const char *id = (const char*)sqlite3_column_text(stmt, 0);
if (id != NULL) {
strncpy(result[i].device_id, id, MAX_DEVICE_ID_LEN - 1);
result[i].device_id[MAX_DEVICE_ID_LEN - 1] = '\0';
}
// 读取时间戳
result[i].timestamp = (time_t)sqlite3_column_int64(stmt, 1);
// 读取CPU使用率
result[i].cpu_usage = sqlite3_column_double(stmt, 2);
// 读取内存使用率
result[i].memory_usage = sqlite3_column_double(stmt, 3);
// 读取温度
result[i].temperature = sqlite3_column_double(stmt, 4);
// 读取状态码
result[i].status_code = sqlite3_column_int(stmt, 5);
i++;//处理下一行数据
}
sqlite3_finalize(stmt);//释放 SQLite 资源
*count = total_count;
return result;//返回查询结果。
}