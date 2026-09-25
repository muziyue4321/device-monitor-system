#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "device_client.h"
#include "network.h"
#include "utils.h"

// 打印使用说明
void print_usage(const char* program_name) {
    printf("Usage: %s <num_devices> <run_time>\n", program_name);
    printf("  num_devices: Number of devices to simulate (1-%d)\n", MAX_DEVICES);
    printf("  run_time:    Simulation time in seconds\n");
    printf("\nExample: %s 5 60\n", program_name);
}

int main(int argc, char* argv[]){
    srand(time(NULL) ^ getpid());

    //检查参数数量是否为3
    if(argc != 3){
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_device = atoi(argv[1]);
    int run_time = atoi(argv[2]);

    if(num_device <= 0 || num_device > MAX_DEVICES){
        fprintf(stderr, "输入的设备数量应该在0到%d\n", MAX_DEVICES);
        print_usage(argv[0]);
    }

    if(run_time <= 5){
    fprintf(stderr, "运行时间不能小于5s\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    run_device_simulation(num_device, run_time);
    return EXIT_SUCCESS;

}