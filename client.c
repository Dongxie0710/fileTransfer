/**
 * @file    client.c
 * @brief   客户端实现
 * @author  lizidong
 * @date    2019/8/6
 * */
#include "client.h"

int main()
{
    menu();
    printf("\n\nok ! it's over !\n");
    return 0;
}

void menu()
{
    printf("**************************************************\n");
    printf("**\tPlease enter the number of your choice\t**\n");
    printf("**\t\t0. 退出系统\t\t\t**\n");
    printf("**\t\t1. 文件服务器探测\t\t**\n");
    printf("**\t\t2. 文件传输(TCP模式)\t\t**\n");
    printf("**\t\t3. 文件传输(UDP模式)\t\t**\n");
    printf("**************************************************\n");
    int op;
    while (scanf("%d", &op) != EOF) 
    {
        switch (op) 
        {
            case 0 :
                printf("已退出系统，欢迎下次登录！\n");
                return ;
            case 1 :
                /*enter the code*/
                printf("fuwuqitance\n");
                break;
            case 2 :
                /*enter the code*/
                printf("tcp\n");
                transFileByTCP();
                break;
            case 3 :
                /*enter the code*/
                printf("udp\n");
                transFileByUDP();
                break;
            default:
                printf("Error ! Please enter number: 0 ~ 3\n"); 
                break;
        }
    }
    return;
}

/**
 * @brief 
 * @param
 * @return
 * */
void transFileByTCP()
{
    printf("----------------------------\n");
    printf("Welcome to tcp module!\n");
    // menu();
    printf("传输完成！请选择您的操作(0~3)\n");
    return ;
}

/**
 * @brief 客户端初始化建立TCP连接
 * @param ser_addr--服务器地址
 * @return 返回socket套接字描述符
 * */
int init_tcp_client(char *ser_addr)
{
    int sockfd; //套接字描述符
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;   //IPv4连接
    server_addr.sin_addr.s_addr = inet_addr(ser_addr);  //服务器地址
    server_addr.sin_port = htons(9877); //使用9877端口

    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        perror("fail to create socket");
        return ERROR;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("fail to connect!");
        return ERROR;
    }

    printf("success to connect!\n");

    return sockfd;
}

/*--------------------以下为UDP传输模式代码实现---------------------------*/
/**
 * 
 * 
 * 
 * */
void transFileByUDP()
{
    return ;
}