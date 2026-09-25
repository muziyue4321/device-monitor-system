#ifndef NETWORK_H
#define NETWORK_H

#include "utils.h"
#include <stdbool.h>

//网络连接函数声明
int connect_to_server(const char *ip, int port);//用与连接服务端
bool send_device_data(int sockfd, const DeviceData *data);//用于向服务端发送模拟的主机信息
bool receive_server_response(int sockfd, char *buffer, int buffer_size);//接收服务端发来的回复信息
void close_connection(int sockfd);//用于关闭与服务器的连接

#endif