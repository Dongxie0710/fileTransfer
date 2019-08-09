/**
 * @file    server.c
 * @brief   服务端实现 
 * @author  lizidong
 * @date    2019/8/6
 * */
#include "server.h"

int main()
{

    return 0;
}


/**
 * @brief 客户端初始化建立TCP连接
 * @param ser_addr--服务器地址
 * @return 返回socket套接字描述符
 * */
int init_tcp_server(char *ser_addr)
{
    int sockfd;
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(9877);

    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        perror("fail to socket");
        return ERROR;
    }

    if (-1 == (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))))
    {
        perror("fail to bind");
        return ERROR;
    }

    if (-1 == (listen(sockfd, 0))) 
    {
        perror("fail to listen");
        return ERROR;
    }

    printf("succeed to create connection!\n");
    return sockfd;
}