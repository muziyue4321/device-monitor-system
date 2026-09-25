#ifndef CONFIG_H
#define CONFIG_H

//服务器配置
#define SERVER_IP "0.0.0.0"//服务器IP,本地巡回地址
#define SERVER_PORT 8888 //服务器端口

#define MAX_DEVICES 50 //定义设备最大数量
#define MAX_DEVICE_ID_LEN 20 //定义设备名称最大长度
#define DATA_BUFFER_SIZE 1024 //定义单次传输数据的最大长度

//定义生成数据的随机范围
//cup占用率 0.0 -- 100.0
#define MIN_CPU_USAGE 0.0
#define MAX_CPU_USAGE 100.0

//内存占用率 0.0 -- 100.0
#define MIN_MEMORY_USAGE 0.0
#define MAX_MEMORY_USAGE 100.0

//温度 25.0 -- 100.0
#define MIN_TEMPERATURE 25.0
#define MAX_TEMPERATURE 100.0

//状态码 0 -- 5
#define MIN_STATUS_CODE 0
#define MAX_STATUS_CODE 5

//发送时间间隔 1000毫秒
#define SEND_INTERVAL_MS 1000

#endif