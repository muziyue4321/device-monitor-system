#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include "network.h"
#include <sys/time.h>

int connect_to_server(const char *ip, int port){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket创建失败！");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0){
        perror("Invalid addr");
        close(sockfd);
        return -1;
    }

    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("连接失败！");
        close(sockfd);
        return -1;
    }

    return sockfd;

}

bool send_device_data(int sockfd, const DeviceData *data){
    char buffer[DATA_BUFFER_SIZE];
    snprintf(buffer, DATA_BUFFER_SIZE, "%s,%ld,%.2f,%.2f,%.2f,%d",
            data->device_id, data->timestamp, data->cpu_usage, 
            data->memory_usage, data->temperature, data->status_code);
    
    printf("Sent:%s\n", buffer);
    int bytes_sent = send(sockfd, buffer, strlen(buffer), 0);
    return (bytes_sent > 0);
}

bool receive_server_response(int sockfd, char *buffer, int buffer_size){
    memset(buffer, 0, buffer_size);
    int byte_receive = recv(sockfd, buffer, buffer_size-1, 0);
    if(byte_receive > 0){
        buffer[buffer_size] = '\0';
        printf("Server Respond:%s\n", buffer);
        return true;
    }
    return false;
}
void close_connection(int sockfd){
    if(sockfd > 0)
        close(sockfd);
}