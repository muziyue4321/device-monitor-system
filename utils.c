#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

double random_double(double min, double max){
    return min + ((double)rand() /  RAND_MAX) * (max - min);
}

int random_int(int min, int max){
    return min + rand() % (max - min + 1); 
}

void generate_device_id(int index, char* buffer){
    snprintf(buffer, MAX_DEVICE_ID_LEN, "DEVICE-%03d", index);
}

void format_timestamp(time_t ts, char *buffer){
    struct tm *tm_info = localtime(&ts);
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
}

void print_device_data(const DeviceData *data){
    char time_str[20];
    format_timestamp(data->timestamp, time_str);
    
    printf("Device: %s\n", data->device_id);
    printf("  Timestamp: %ld (%s)\n", data->timestamp, time_str);
    printf("  CPU Usage: %.2f%%\n", data->cpu_usage);
    printf("  Memory Usage: %.2f%%\n", data->memory_usage);
    printf("  Temperature: %.2f°C\n", data->temperature);
    printf("  Status Code: %d\n", data->status_code);
}

void generate_device_status(DeviceData *data, const char *device_id){
    strncpy(data->device_id, device_id, MAX_DEVICE_ID_LEN - 1);
    data->device_id[MAX_DEVICE_ID_LEN - 1] = '\0';
    data->timestamp = time(NULL);
    data->cpu_usage = random_double(MIN_CPU_USAGE, MAX_CPU_USAGE);
    data->memory_usage = random_double(MIN_MEMORY_USAGE, MAX_MEMORY_USAGE);
    data->temperature = random_double(MIN_TEMPERATURE, MAX_TEMPERATURE);
    data->status_code = random_int(MIN_STATUS_CODE, MAX_STATUS_CODE);
}