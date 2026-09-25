#ifndef DATABASE_SERVER_H
#define DATABASE_SERVER_H
#include <stdbool.h>
#include "utils.h"
// 数据库初始化函数
bool init_database(void);
// 关闭数据库连接
void close_database(void);
// 插入设备数据到数据库
bool insert_device_data(const DeviceData *data);
// 获取所有设备的最新状态
DeviceData* get_all_devices_latest_status(int* count);
// 获取特定设备的所有历史数据
DeviceData* get_device_history(const char* device_id, int* count);
#endif