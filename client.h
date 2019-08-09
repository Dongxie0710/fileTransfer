/**
 * @file    client.h
 * @brief   客户端变量、函数声明 
 * @author  lizidong
 * @date    2019/8/6
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ERROR -1
#define BUFFSIZE 2048   /*缓冲区长度 */
#define TCP_PORT 9877   /*TCP传输端口 */
#define UDP_PORT 9877   /*UDP传输端口 */

/*以下内容为函数声明*/
void menu();
void transFileByTCP();
/* 初始化客户端建立TCP socket连接 */
int init_tcp_client(char *ser_addr);
/*TCP模式客户端上传文件至服务器 */
int client_upload_tcp(char* filename);
void transFileByUDP();