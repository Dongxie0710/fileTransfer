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

#define ERROR -1
#define BUFFSIZE 2048   /*缓冲区长度 */
#define TCP_PORT 9877   /*TCP传输端口 */
#define UDP_PORT 9877   /*UDP传输端口 */

/* 初始化服务器端 */
int init_tcp_server(void);