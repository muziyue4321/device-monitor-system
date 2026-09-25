#ifndef UTILS_H
#define UTILS_H 
#include "config.h"
#include <time.h>

//定义设备数据结构
/*从上到下以此为： 设备id
                  时间戳，标志信息生成时间
                  cpu占用率
                  内存占用率
                  温度  
                  状态码
*/
typedef struct{
    char device_id[MAX_DEVICE_ID_LEN];
    time_t timestamp;
    double cpu_usage;
    double memory_usage;
    double temperature;
    int status_code;
}DeviceData;

//工具函数声明
double random_double(double min, double max);//用于生成随机浮点数
int random_int(int min, int max);//用于生成随机整数
void generate_device_id(int index, char* buffer);//用于生成设备名称
void format_timestamp(time_t ts, char *buffer);//用于生成时间戳，并转换为字符串存储
void print_device_data(const DeviceData *data);//用于打印设备信息
void generate_device_status(DeviceData *data, const char *device_id);//将上述函数集成在这一个函数，统一生成随机设备参数

#endif 