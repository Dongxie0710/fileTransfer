/**
 * @file    server.h
 * @brief   服务端变量、函数声明
 * @author  lizidong
 * @date    2019/8/6
 * */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define ERROR -1
#define BUFFSIZE 1024                   /* 缓冲区大小 */
#define UDP_MCAST_ADDR "224.0.0.100"    /* 一个局部连接多播地址，路由器不进行转发 */
#define UDP_MCAST_PORT 9875             /* 多播端口 需要保证客户端和服务端一致 */

#define TCP_TRANS_PORT 9877             /* TCP传输端口 需要保证客户端和服务端一致 */

int UDP_TRANS_PORT = 9877;             /* UDP传输端口 需要保证客户端和服务端一致 */

/* 服务器加入组播组，UDP实现响应组播信息 */
int respon_mulcast_init();
/* 服务端TCP模式初始化，建立socket连接 */
int init_tcp_server(void);
/* 服务端UDP连接初始化，建立socket连接 */
int init_udp_server();

/* 监听TCP传输端口，等待客户端TCP连接 */
void wait_tcp_connect(void* param);
/* TCP连接处理线程，acceptTCP客户端后创建 */
void deal_recv_tcp(void* param);
/* 根据消息类型对收到的TCP消息进行处理 */
void handle_tcp_recv_msg(int sockfd, char* str);
/* 处理收到的ls命令,TCP传输 */
void ls_file(int sockfd);
/* 处理TCP模式文件下载操作 */
void tcp_download_func(int sockfd, char msg[][BUFFSIZE]);
/* 处理TCP模式文件上传操作 */
void tcp_upload_func(int sockfd, char msg[][BUFFSIZE]);

/* 监听UDP传输端口，等待客户端UDP连接 */
void wait_udp_connect(void* param);
/* UDP连接处理线程 */
void deal_recv_udp(void* param);
/* 根据消息类型对收到的UDP消息进行处理 */
void handle_udp_recv_msg(int sockfd, struct sockaddr_in client_addr, char* str);
/* 处理UDP模式文件下载操作 */
void udp_download_func(int sockfd, struct sockaddr_in client_addr, char msg[][BUFFSIZE]);
/* 处理UDP模式文件上传操作 */
void udp_upload_func(int sockfd, struct sockaddr_in client_addr, char msg[][BUFFSIZE]);
