/**
 * @file    server.h
 * @brief   ����˱�������������
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
#define BUFFSIZE 1024                   /* ��������С */
#define UDP_MCAST_ADDR "224.0.0.100"    /* һ���ֲ����Ӷಥ��ַ��·����������ת�� */
#define UDP_MCAST_PORT 9875             /* �ಥ�˿� ��Ҫ��֤�ͻ��˺ͷ����һ�� */

#define TCP_TRANS_PORT 9877             /* TCP����˿� ��Ҫ��֤�ͻ��˺ͷ����һ�� */

int UDP_TRANS_PORT = 9877;             /* UDP����˿� ��Ҫ��֤�ͻ��˺ͷ����һ�� */

/* �����������鲥�飬UDPʵ����Ӧ�鲥��Ϣ */
int respon_mulcast_init();
/* �����TCPģʽ��ʼ��������socket���� */
int init_tcp_server(void);
/* �����UDP���ӳ�ʼ��������socket���� */
int init_udp_server();

/* ����TCP����˿ڣ��ȴ��ͻ���TCP���� */
void wait_tcp_connect(void* param);
/* TCP���Ӵ����̣߳�acceptTCP�ͻ��˺󴴽� */
void deal_recv_tcp(void* param);
/* ������Ϣ���Ͷ��յ���TCP��Ϣ���д��� */
void handle_tcp_recv_msg(int sockfd, char* str);
/* �����յ���ls����,TCP���� */
void ls_file(int sockfd);
/* ����TCPģʽ�ļ����ز��� */
void tcp_download_func(int sockfd, char msg[][BUFFSIZE]);
/* ����TCPģʽ�ļ��ϴ����� */
void tcp_upload_func(int sockfd, char msg[][BUFFSIZE]);

/* ����UDP����˿ڣ��ȴ��ͻ���UDP���� */
void wait_udp_connect(void* param);
/* UDP���Ӵ����߳� */
void deal_recv_udp(void* param);
/* ������Ϣ���Ͷ��յ���UDP��Ϣ���д��� */
void handle_udp_recv_msg(int sockfd, struct sockaddr_in client_addr, char* str);
/* ����UDPģʽ�ļ����ز��� */
void udp_download_func(int sockfd, struct sockaddr_in client_addr, char msg[][BUFFSIZE]);
/* ����UDPģʽ�ļ��ϴ����� */
void udp_upload_func(int sockfd, struct sockaddr_in client_addr, char msg[][BUFFSIZE]);
