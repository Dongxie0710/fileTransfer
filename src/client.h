/**
 * @file    client.h
 * @brief   �ͻ���ͷ�ļ�����������
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

#define ERROR -1                    /* ����-1 */
#define BUFFSIZE 1024               /* ��������С */
#define MCAST_DATA "Detect server"  /* �鲥������Ϣ���� */
#define MCAST_ADDR "224.0.0.100"    /* һ���ֲ����Ӷಥ��ַ��·����������ת�� */
#define MCAST_PORT 9875             /* �ಥ�˿� ��Ҫ��֤�ͻ��˺ͷ����һ�� */

#define TCP_TRANS_PORT 9877         /* TCP����˿� ��Ҫ��֤�ͻ��˺ͷ����һ�� */
#define UDP_TRANS_PORT 9878         /* UDP����˿� ��Ҫ��֤�ͻ��˺ͷ����һ�� */

#define ERROR_NO_INPUT -1           /* ������ */
#define ERROR_INVALID_INPUT -2      /* �Ƿ����� */

#define MAX_CMD_NUM 8               /* ϵͳ�������������8�� */

char cmd[3][64];                    /* ����ָ�� */
/* ����ϵͳָ�� */
char* SysCommand[] = {"help", "detect", "connect", "ls_client", "ls_server",
                        "download", "upload", "quit"};

/* ����ϵͳ���� */
enum Sys_Command
{
    Sys_Help,       /* ��ӡ�����˵� */
    Sys_Detect,     /* ������̽������ */
    Sys_Connect,    /* ���ӷ����� */
    Sys_Ls_Client,  /* ��ӡ�ͻ����ļ��б� */
    Sys_Ls_Server,  /* ��ӡ������ļ��б� */
    Sys_Download,   /* �����ļ� */
    Sys_Upload,     /* �ϴ��ļ� */
    Sys_Quit,       /* �˳�ϵͳ */
};


/* �ļ���Ϣ�ṹ�� */
struct FileInfo
{
    unsigned int fileSize;  /* �ļ���С */
    char fileName[256];     /* �ļ��� */
    char hashCode[41];      /* hashУ���� */
};

/*************************����Ϊ��������***************************/

/* ��ӡ�����˵� */
void menu();
/* �������룬�������� */
int handle_input(char* str);


/* ������̽��ģ�飬UDP�鲥��ʽ̽������� */
int detectServer();
/* ��ӡ���UDP̽������λ�����߳��� */
void print_detect_result(void* param);

/* �ͻ���connectģ��ӿ� */
void connect_client(char cmd_str[][64]);
/* �ͻ��˳�ʼ������TCP���� */
int init_tcp_client(char *ip_addr);
/* �ͻ��˳�ʼ������UDP���� */
int init_udp_client(char *ip_addr);

/* ��ȡ�ͻ��˿������ļ��б� */
void ls_file_client();
/* ��ȡ����˿������ļ��б� */
void ls_file_server();

/* �ͻ����ϴ�ģ��ʵ�ֽӿ� */
void interface_upload_client(char cmd_str[][64]);
/* �ͻ����ϴ�ģ��ʵ�ֽӿ� */
void my_interface_upload_client(char cmd_str[][64]);
/* �ͻ���TCPģʽ���ϴ��ļ��������� */
int client_upload_tcp(char* filename, char* singlefilename);
/* �ͻ���UDPģʽ���ϴ��ļ��������� */
int client_upload_udp(char* filename, char* singlefilename);

/* �ͻ�������ģ��ʵ�ֽӿ� */
void interface_download_client(char cmd_str[][64]);
/* TCPģʽ�´ӷ����������ļ� */
int client_download_tcp(char* filename);
/* UDPģʽ�´ӷ����������ļ� */
int client_download_udp(char* filename);
