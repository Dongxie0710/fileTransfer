/**
 * @file    client.h
 * @brief   客户端头文件，函数声明
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
#include <pthread.h>

#define ERROR -1                    /* 返回-1 */
#define BUFFSIZE 1024               /* 缓冲区大小 */
#define MCAST_DATA "Detect server"  /* 组播发送消息内容 */
#define MCAST_ADDR "224.0.0.100"    /* 一个局部连接多播地址，路由器不进行转发 */
#define MCAST_PORT 9875             /* 多播端口 需要保证客户端和服务端一致 */

#define TCP_TRANS_PORT 9877         /* TCP传输端口 需要保证客户端和服务端一致 */
#define UDP_TRANS_PORT 9878         /* UDP传输端口 需要保证客户端和服务端一致 */

#define ERROR_NO_INPUT -1           /* 空输入 */
#define ERROR_INVALID_INPUT -2      /* 非法输入 */

#define MAX_CMD_NUM 8               /* 系统命令数，最多有8个 */

char cmd[3][64];                    /* 工作指令 */
/* 保存系统指令 */
char* SysCommand[] = {"help", "detect", "connect", "ls_client", "ls_server",
                        "download", "upload", "quit"};

/* 定义系统命令 */
enum Sys_Command
{
    Sys_Help,       /* 打印导航菜单 */
    Sys_Detect,     /* 服务器探测命令 */
    Sys_Connect,    /* 连接服务器 */
    Sys_Ls_Client,  /* 打印客户端文件列表 */
    Sys_Ls_Server,  /* 打印服务端文件列表 */
    Sys_Download,   /* 下载文件 */
    Sys_Upload,     /* 上传文件 */
    Sys_Quit,       /* 退出系统 */
};


/* 文件信息结构体 */
struct FileInfo
{
    unsigned int fileSize;  /* 文件大小 */
    char fileName[256];     /* 文件名 */
    char hashCode[41];      /* hash校验码 */
};

/*************************以下为函数声明***************************/

/* 打印帮助菜单 */
void menu();
/* 处理输入，解析命令 */
int handle_input(char* str);


/* 服务器探测模块，UDP组播方式探测服务器 */
int detectServer();
/* 打印输出UDP探测结果，位于子线程中 */
void print_detect_result(void* param);

/* 客户端connect模块接口 */
void connect_client(char cmd_str[][64]);
/* 客户端初始化建立TCP连接 */
int init_tcp_client(char *ip_addr);
/* 客户端初始化建立UDP连接 */
int init_udp_client(char *ip_addr);

/* 获取客户端可下载文件列表 */
void ls_file_client();
/* 获取服务端可下载文件列表 */
void ls_file_server();

/* 客户端上传模块实现接口 */
void interface_upload_client(char cmd_str[][64]);
/* 客户端上传模块实现接口 */
void my_interface_upload_client(char cmd_str[][64]);
/* 客户端TCP模式下上传文件至服务器 */
int client_upload_tcp(char* filename, char* singlefilename);
/* 客户端UDP模式下上传文件至服务器 */
int client_upload_udp(char* filename, char* singlefilename);

/* 客户端下载模块实现接口 */
void interface_download_client(char cmd_str[][64]);
/* TCP模式下从服务器下载文件 */
int client_download_tcp(char* filename);
/* UDP模式下从服务器下载文件 */
int client_download_udp(char* filename);
