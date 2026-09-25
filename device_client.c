#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "device_client.h"
#include "network.h"
#include "utils.h"

void* device_simulator_thread(void *arg){
    DeviceThreadArgs* args = (DeviceThreadArgs*)arg;
    time_t start_time = time(NULL);//标注开始运行时间
    time_t current_time;//标注实时时间
    DeviceData data;

    printf("Device %s try to connect to server!\n", args->device_id);
    args->sockfd = connect_to_server(SERVER_IP, SERVER_PORT);
    
    if(args->sockfd < 0){
        fprintf(stderr, "Device %s failed to connect to server!\n", args->device_id);
        free(args);
        pthread_exit(NULL);
    }

    printf("Device %s has connected to server!\n", args->device_id);

    do{
        //生成设备状态参数
        generate_device_status(&data, args->device_id);

        //向服务端发送设备信息
        if(send_device_data(args->sockfd, &data)){
            char buffer_response[DATA_BUFFER_SIZE];
            //发送成功后接收服务端发来的回应信息
            receive_server_response(args->sockfd, buffer_response, DATA_BUFFER_SIZE);
        }else{
            fprintf(stderr, "Device %s faile to send data to Server!\n",args->device_id);
        }
        
        //等待一秒，将时间错开
        sleep(1);
        current_time = time(NULL);

    }while(difftime(current_time, start_time) < args->running_time);
    
    printf("Device %s simulation completed.\n", args->device_id);
    
    free(args);
    pthread_exit(NULL);
}

void run_device_simulation(int num_device, int run_time){
    pthread_t threads[num_device];
    DeviceThreadArgs *args[num_device];
    printf("starting %d device simulators for %d second!\n", num_device, run_time);
    
    for(int i = 0;i < num_device; i++){
        args[i] = (DeviceThreadArgs *)malloc(sizeof(DeviceThreadArgs));
        
        if(!args[i]){
            perror("分配内存失败");
            exit(EXIT_FAILURE);
        }
        
        //生成主机名
        generate_device_id(i, args[i]->device_id);
        args[i]->running_time = run_time;
        //设置随机延迟0-200ms之间
        usleep(random_int(0,200000));
        
        if(pthread_create(&threads[i], NULL, device_simulator_thread, args[i]) != 0){
            perror("线程创建失败");
            free(args[i]);
            exit(EXIT_FAILURE);
        }
    }

    //等待所有线程结束
    for(int i = 0; i < num_device; i++){
        pthread_join(threads[i], NULL);
    }

    printf("Simulation completed after %d seconds.\n", run_time);
}