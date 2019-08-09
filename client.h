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
/*以下内容为函数声明*/
void menu();
void transFileByTCP();
int init_tcp_client(char *ser_addr);
void transFileByUDP();