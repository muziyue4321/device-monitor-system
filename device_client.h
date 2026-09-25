#ifndef DEVICE_CLIRNT_H
#define DEVICE_CLIRNT_H
#include <pthread.h>
#include "config.h"
/*设置线程结构体
从上到下为：设备名称
          运行时间
          套接字  
          线程号*/
typedef struct
{
    char device_id[MAX_DEVICE_ID_LEN];
    int running_time;                 
    int sockfd;                       
    pthread_t thread_id;              

}DeviceThreadArgs;

void* device_simulator_thread(void *arg);//设备在线程中使用的线程，将包含连接，生成随机数，发送，接受，以及关闭连接的作用
void run_device_simulation(int num_device, int run_time);//运行设备的模拟运行

#endif