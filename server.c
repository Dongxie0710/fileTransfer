/**
 * @file    server.c
 * @brief   ����˺���ʵ�� 
 * @author  lizidong
 * @date    2019/8/6
 * */
#include "server.h"
#include "tools.h"

char ser_place_of_file[BUFFSIZE];   /* ��ŷ�����ļ��洢·�� */

int main(int argc, char **argv)
{
    /* Ĭ����Դ�ļ�ͬĿ¼�´���serverFile�ļ��� */
    strcpy(ser_place_of_file, "./serverFile/");
    mk_folder_of_file(ser_place_of_file);

    /* UDP�鲥�����߳� */
    pthread_t mulcast_thread;
    pthread_create(&mulcast_thread, NULL, (void *)respon_mulcast_init, NULL);

    /* TCP����˿ڼ����߳� */
    int tcp_trans_sockfd = init_tcp_server();
    if (tcp_trans_sockfd > 0)
    {
        pthread_t tcp_trans_thread;
        pthread_create(&tcp_trans_thread, NULL, (void *)wait_tcp_connect, &tcp_trans_sockfd);
    }

    int udp_trans_sockfd = init_udp_server();
    if (udp_trans_sockfd > 0)
    {
        pthread_t udp_trans_thread;
        pthread_create(&udp_trans_thread, NULL, (void *)wait_udp_connect, &udp_trans_sockfd);
    }

    /* ���߳� */
    char buf[BUFFSIZE];
    while (1)
    {
        scanf("%s", buf);
    }
    // return 0;
}


/**
 * @brief �������˼����鲥����Ӧ�鲥��Ϣ
 * @param void
 * @return socket�׽��ֱ�ʶ��
 * */
int respon_mulcast_init()
{
    /* �½�socket ���ڽ���UDP�鲥��Ϣ����Ӧ������̽�� */
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        perror("fail to create socket(udp_respon_detect)");
        return ERROR;
    }
    
    /* ��ʼ����ַΪ0 */
    struct sockaddr_in my_mulcast_addr;
    memset(&my_mulcast_addr, 0, sizeof(my_mulcast_addr));   /* ��0��� */
    my_mulcast_addr.sin_family = AF_INET;
    my_mulcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_mulcast_addr.sin_port = htons(UDP_MCAST_PORT);

    /* ���׽��ֺͶ˿� */
    if (-1 == bind(sockfd, (struct sockaddr *)&my_mulcast_addr, sizeof(my_mulcast_addr)))
    {
        perror("fail to bind (udp_mulcast)");
        return ERROR;
    }

    /* ��ʼ���鲥�ṹ����� */
    struct ip_mreq mreq;
    memset(&mreq, 0x00, sizeof(struct ip_mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(UDP_MCAST_ADDR);  /* �����鲥��ַ */
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);  /* ��������ӿڵ�ַ��Ϣ */
    
    /* �����鲥�� */
    if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)))
    {
        perror("fail to add membership (join the mulcast)");
        return ERROR;
    }

    /* ��ֹ���ݻش������ػ��ؽӿ� */
    unsigned char loop = 0;
    if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)))
    {
        perror("Error: set data loop");
        return ERROR;
    }


    struct sockaddr_in cli_addr;    /* ����ͻ�����Ϣ */
    char recv_msg[BUFFSIZE + 1];    /* ������Ϣ���� */
    unsigned int recv_size = 0;     /* ʵ���յ�����Ϣ��С */
    unsigned int addr_size = sizeof(struct sockaddr_in);
    /* ѭ�������鲥��Ϣ */
    while (1)
    {
        memset(recv_msg, 0x00, BUFFSIZE + 1);
        recv_size = recvfrom(sockfd, recv_msg, BUFFSIZE, 0, (struct sockaddr *)&cli_addr, &addr_size);
        if (recv_size < 0)
        {
            perror("recvfrom error (udp_detect)");
            return ERROR;
        }
        else 
        {
            recv_msg[recv_size] = '\0'; /* �ɹ��������ݱ���ĩβ�ӽ����� */

            /* ��ȡip��port */
            char ip_str[INET_ADDRSTRLEN];   /* IPv4��ַ��Ĭ��Ϊ16 */
            inet_ntop(AF_INET, &(cli_addr.sin_addr), ip_str, sizeof(ip_str));
            int port = ntohs(cli_addr.sin_port);
            
            /* ��ӡ������յ�̽����Ϣ */
            printf("Received from %s--%d : [%s]\n", ip_str, port, recv_msg);
            /* ��ͻ��˻ظ����ݣ����� */
            char response[BUFFSIZE + 1] = "detect response, ok!";
            if (sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&cli_addr, addr_size) < 0)
            {
                perror("sendto error (response for detect)");
                return ERROR;
            }
        }
    }

}

/**
 * @brief �����TCPģʽ��ʼ��������socket����
 * @param void
 * @return socket�׽��ֱ�ʶ��
 * */
int init_tcp_server(void)
{
    int sockfd;
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TCP_TRANS_PORT);

    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        perror("\tfail to create tcp socket");
        return ERROR;
    }

    if (-1 == (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))))
    {
        perror("\tfail to bind");
        return ERROR;
    }

    int backlog = 10000;    /* ά��listen���д�СΪ10000 */
    if (-1 == (listen(sockfd, backlog))) 
    {
        perror("\tfail to listen");
        return ERROR;
    }

    // printf("succeed to init tcp socket!\n");
    return sockfd;
}

/**
 * @brief �����UDP���ӳ�ʼ��������socket����
 * @param void
 * @return socket�׽��ֱ�ʶ��sockfd
 * */
int init_udp_server()
{
    int sockfd;
    if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        perror("\tfail to create udp socket");
        return ERROR;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(UDP_TRANS_PORT);
    /* �󶨶˿� */
    if (-1 == bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        perror("\tfail to bind udp socket");
        return ERROR;
    }

    return sockfd;
}

/**
 * @brief ����TCP����˿ڣ��ȴ��ͻ���TCP����
 *         �ɹ�acceptһ��tcp���Ӻ󣬴���һ���߳�ȥ����
 * @param server��TCP����sockfd
 * @return void
 * */
void wait_tcp_connect(void* param)
{
    int serv_sockfd = *((int *)param);
    while (1)
    {
        int cli_sockfd;
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        int addr_size = sizeof(client_addr);
        if (-1 == (cli_sockfd = accept(serv_sockfd, (struct sockaddr *)&client_addr, &addr_size)))
        {
            perror("\ttcp accept error");
            return ;
        }

        /* �ɹ�acceptһ��tcp�ͻ��ˣ�Ϊ�䴴��һ��������Ϣ�߳� */
        pthread_t tcp_recv_thread;
        pthread_create(&tcp_recv_thread, NULL, (void *)deal_recv_tcp, &cli_sockfd);
        /* ���÷���̬��ʹÿ���߳��˳����Զ��ͷ���Դ */
        pthread_detach(tcp_recv_thread);
    }
}

/**
 * @brief TCP���Ӵ����̣߳�acceptTCP�ͻ��˺󴴽�
 * @param �ͻ���TCP����sockfd
 * @return void
 * */
void deal_recv_tcp(void* param)
{
    int cli_sockfd = *((int *)param);
    char buff[BUFFSIZE + 1] = {0};  /* ��ʼ������Ϊ0 */
    int recv_size;
    while (1)
    {
        recv_size = recv(cli_sockfd, buff, BUFFSIZE, 0);
        if (recv_size <= 0)
        {
            /* �ر�socket�˳��߳� �����÷���̬���˳����ͷ���Դ */
            close(cli_sockfd);  /* �������socket����close_wait״̬ */
            pthread_exit(0);
            return ;
        }
        buff[recv_size] = '\0';
        handle_tcp_recv_msg(cli_sockfd, buff);
    }
}

/**
 * @brief ������Ϣ���Ͷ��յ���TCP��Ϣ���д���
 * @param �ͻ���TCP����sockfd   message�ַ���
 * @return void
 * */
void handle_tcp_recv_msg(int sockfd, char* str)
{
    /* �ϴ�ָ�� �ָ�str �� msg_list������ݣ�msg_type, filename, filesize, hashcode */
    /* ����ָ�� �ָ�str �� msg_list������ݣ�msg_type, filename, fileset, hashcode */
    int i = 0;
    // char seg[] = ",";
    char msg_list[4][BUFFSIZE] = {0};
    char *substr = strtok(str, ",");    /* �á������ָ��ַ���str */
    while (NULL != substr)
    {
        strcpy(msg_list[i++], substr);
        substr = strtok(NULL, ",");
    }

    switch (atoi(msg_list[0]))
    {
        case MSG_TCP_LS :
            ls_file(sockfd);
            break;
        case MSG_TCP_DOWNLOAD :
            tcp_download_func(sockfd, msg_list);
            break;
        case MSG_TCP_UPLOAD :
            tcp_upload_func(sockfd, msg_list);
            break;
        default :
            break;
    }
}

/**
 * @brief �����յ���ls����,TCP����
 * @param sockfd
 * @return void
 * */
void ls_file(int sockfd)
{
    DIR *dir;
    struct dirent *ptr;

    char respon_ls[2 * BUFFSIZE] = {0};
    strcpy(respon_ls, "file list in server as follow:\n");
    char default_respon_ls[2 * BUFFSIZE];
    strcpy(default_respon_ls, "No available file in server ! \n");

    if (NULL == (dir = opendir(ser_place_of_file)))
    {
        send(sockfd, default_respon_ls, strlen(default_respon_ls), 0);
        return ;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        /* current dir or parrent dir */
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
        {
            continue;
        }
        else if (ptr->d_type == 8)  /* file */
        {
            sprintf(respon_ls, "%s\r\n%s", respon_ls, ptr->d_name);
        }
        else
        {
            continue;
        }
        
    }
    closedir(dir);

    if (0 == strcmp(respon_ls, "file list in server as follow:\n"))
    {
        send(sockfd, default_respon_ls, strlen(default_respon_ls), 0);
    }
    else
    {
        send(sockfd, respon_ls, strlen(respon_ls), 0);
    }
      
}

/**
 * @brief ����TCPģʽ�ļ����ز���
 *        respon_msg--"flag, hash, size"
 * @param sockfd--tcp�����׽��������� msg[][]--���յ�����Ϣ
 * @return void
 * */
void tcp_download_func(int sockfd, char msg[][BUFFSIZE])
{
    int file_set = atoi(msg[2]);    /* �Ӹ�λ�ü�������,�ϵ� */
    char save_file[BUFFSIZE] = {0}; /* �ļ����λ�� */
    sprintf(save_file, "%s%s", ser_place_of_file, msg[1]);
    char respon_msg[BUFFSIZE] = {0};
    strcpy(respon_msg, "0,0,0");    /* Ĭ��ֵ���ļ������ڣ� */
    /* �ļ������� */
    if (-1 == access(save_file, F_OK))
    {
        send(sockfd, respon_msg, BUFFSIZE, 0);
    }
    else
    {
        struct stat s_buf;
        stat(save_file, &s_buf);    /* ��ȡ��Դ��Ϣ */
        if (S_ISDIR(s_buf.st_mode))
        {
            /* Ŀ¼ */
            send(sockfd, respon_msg, BUFFSIZE, 0);
            return ;
        }

        FILE *fp;
        char hashCode[41];  /* �������Ĺ�ϣֵ */
        /* �����䣬����ļ�У�麯�� */
        strcpy(hashCode, MY_FAKE_HASHCODE);
        /* msg[3]--hashcode, ��Ҫ���� */
        if (0 == strcmp(hashCode, msg[3]))
        {
            sprintf(respon_msg, "%d,%s,%d", 2, hashCode, get_file_size(save_file));
            send(sockfd, respon_msg, BUFFSIZE, 0);
        }
        /* ���������񣬲���Ҫ���� */
        else 
        {
            sprintf(respon_msg, "%d,%s,%d", 1, hashCode, get_file_size(save_file));
            send(sockfd, respon_msg, BUFFSIZE, 0);
            file_set = 0;   /* ��Ը��ǵ���� */
        }

        fp = fopen(save_file, "rb");
        char buf[BUFFSIZE + 1] = {0};
        fseek(fp, file_set, SEEK_SET);  /* ��λ���ͻ����ϴ���ƫ��λ�� */
        int readSize = 0;   /* ��ȡ���� */
        int sendSize = 0;   /* ���ͳ��� */
        int totalTrans = 0; /* ��¼�Ѿ������˶��� */
        while (1)
        {
            readSize = fread(buf, 1, BUFFSIZE, fp);
            if (0 == readSize)
            {
                break;
            }
            /* �ͻ��˶Ͽ����������һ��ʧЧ��socket��������ʱ���ײ��׳�SIGIPE�ź�
            * ���ź�ȱʡ����ʽΪ�˳����������̣�����MSG_NOSIGNAL���Ը��ź�
            * */
            sendSize = send(sockfd, (const char *)buf, readSize, MSG_NOSIGNAL);
            if (-1 == sendSize)
            {
                /* ���ͳ����ͻ����쳣�Ͽ� */
                /* �ϵ��������������ݽ��ն�ʵ�� */
                printf("\tDisconnect to client!\n");
                fclose(fp);
                return ;
            }
            totalTrans += sendSize;
        }
        printf("Complete send! Total trans %d bytes\n", totalTrans);
        fclose(fp);
    }
    
}

/**
 * @brief ����TCPģʽ�ļ��ϴ�����
 * @param sockfd--tcp�����׽��������� msg[][]--���յ�����Ϣ
 * @return void
 * */
void tcp_upload_func(int sockfd, char msg[][BUFFSIZE])
{
    int flag = 0;   /* Ĭ���������� */
    int bp_size = 0;    /* ����fseek��λ�ϵ� */

    /* �����Ƿ���ڶϵ��ļ� */
    char bp_file[BUFFSIZE];
    sprintf(bp_file, "%s.%s.bot.break", ser_place_of_file, msg[1]);
    char tmp_zero[BUFFSIZE];    /* ������ͻ��˷��� */
    strcpy(tmp_zero, "0");
    if (access(bp_file, F_OK) != -1)
    {
        /* �ļ����� */
        FILE *fp_bp;
        fp_bp = fopen(bp_file, "r");
        char tmp_size[BUFFSIZE];
        char tmp_hashcode[BUFFSIZE];
        fgets(tmp_size, BUFFSIZE, fp_bp);
        tmp_size[strlen(tmp_size) - 1] = '\0';
        fgets(tmp_hashcode, BUFFSIZE, fp_bp);
        fclose(fp_bp);
        if (0 == strcmp(tmp_hashcode, msg[3]))
        {
            /* �ļ����͹�ϣֵ��ͬ������ */
            flag = 1;
            bp_size = atoi(tmp_size);
            send(sockfd, tmp_size, BUFFSIZE, 0);
            remove(bp_file);
        }
        else
        {
            /* ͬ����ֱ�Ӹ��� */
            send(sockfd, tmp_zero, BUFFSIZE, 0);
        }
    }
    else
    {
        /* �ļ������� */
        send(sockfd, tmp_zero, BUFFSIZE, 0);
    }

    FILE *fp;
    char save_file[BUFFSIZE];   /* �洢·��+�ļ��� */
    sprintf(save_file, "%s%s", ser_place_of_file, msg[1]);
    if (0 == flag)   
    {
        /* ����Ҫ���� */
        fp = fopen(save_file, "wb+");    /* �����ƴ򿪣� ����Դ�ļ� */
    }
    else
    {
        fp = fopen(save_file, "rb+");   /* �����ƴ򿪣� ������Դ�ļ� */
    }
    
    int totalBytes = 0; /* ����ͳ�����ֽ��� */
    fseek(fp, bp_size, SEEK_SET);   /* ��ת��ָ��λ�ã��ϵ㣩 */
    while (1)
    {
        char buf[BUFFSIZE] = {0};
        int len = recv(sockfd, (char *)buf, BUFFSIZE, 0);
        if (0 == len)
        {
            /* �ͻ����쳣�Ͽ� */
            fclose(fp);

            /* �����ϵ��ļ��� ��ʽΪ.filename.bot.break ���أ����û�͸�� */
            FILE *fp_break;
            char break_file[BUFFSIZE];
            sprintf(break_file, "%s.%s.bot.break", ser_place_of_file, msg[1]);
            /* ��ͬ���ļ��Ķϵ�������ֻ��������������ǰ��ı������ͬ���ļ����� */
            fp_break = fopen(break_file, "w+");
            char str_totalBytes[BUFFSIZE];
            sprintf(str_totalBytes, "%d", totalBytes);
            fwrite(str_totalBytes, 1, strlen(str_totalBytes), fp_break);    /* д���ѽ��ճ��� */
            fwrite("\r\n", 1, 2, fp_break); /* ���� */
            fwrite(msg[3], 1, strlen(msg[3]), fp_break);  /* д��hashcode */
            fclose(fp_break);
            break;
        }

        fwrite(buf, 1, len, fp);

        totalBytes += len;
        /* д���ֽ�����ͬ�������ܴ��ڰ����۸ĵ��������У��hashcode */
        if (totalBytes == (atoi(msg[2]) - bp_size))
        {
            fclose(fp);
            char hashCode[41];
            char respons_msg[BUFFSIZE];
            /* �ļ�У��ģ���ʵ�� */
            // strcpy(hashCode, get_sha1_from_file(save_file)); /* ���ؼ���д����ɵ��ļ���hashcode */
            strcpy(hashCode, MY_FAKE_HASHCODE);
            
            if (0 == strcmp(hashCode, msg[3]))
            {
                /* ���������� */
                if (0 == flag)   
                {
                    sprintf(respons_msg, "File Complete True; Sha1: %s", hashCode);
                }
                /* �������� */
                else
                {
                    sprintf(respons_msg, "Continue File Complete True; Sha1: %s", hashCode);
                }
                send(sockfd, respons_msg, BUFFSIZE, 0);
            }
            else
            {
                /* ���������� */
                if (0 == flag)   
                {
                    sprintf(respons_msg, "File Complete False; Sha1: %s", hashCode);
                }
                /* �������� */
                else
                {
                    sprintf(respons_msg, "Continue File Complete False; Sha1: %s\n", hashCode);
                }
                send(sockfd, respons_msg, BUFFSIZE, 0);
            }
            
            break;
        }
    }
    
}

/**
 * @brief ����UDP����˿ڣ��ȴ��ͻ���UDP����
 *         Ϊÿ���ͻ�����һ���߳�ȥ����ÿ���ͻ���Ӧһ���˿�
 * @param server��UDP����sockfd
 * @return void
 * */
void wait_udp_connect(void* param)
{
    int serv_sockfd = *((int *)param);
    while (1)
    {
        char tmp_msg[BUFFSIZE] = {0};   /* ������Կͻ��˵���Ϣ�������������� */

        // int cli_sockfd;
        struct sockaddr_in client_addr;
        struct sockaddr_in *tmp_client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        int addr_size = sizeof(client_addr);

        int re = recvfrom(serv_sockfd, tmp_msg, BUFFSIZE, 0, (struct sockaddr *)&client_addr, &addr_size);
        if (-1 == re)
        {
            perror("recvfrom error");
            return ;
        }
        
        tmp_client_addr = malloc(sizeof(struct sockaddr_in));   /* �߳̽ṹ����Ҫ���䵽���� */
        *tmp_client_addr = client_addr;

        /* ���÷���̬��ʹÿ���߳����˳����Զ��ͷ���Դ */
        pthread_t udp_recv_thread;
        pthread_create(&udp_recv_thread, NULL, (void *)deal_recv_udp, tmp_client_addr);
        pthread_detach(udp_recv_thread);
    }
}

/**
 * @brief Ϊÿ��udp�ͻ��˴���һ�������߳�
 * @param ��UDP�ͻ��˵�sockaddr_in�ṹ��
 * @return void
 * */
void deal_recv_udp(void* param)
{
    struct sockaddr_in client_addr = *((struct sockaddr_in *)param);
    free(param);

    /* �����µ�sockfd, Ϊÿ��UDP�ͻ��˷����µĶ˿� */
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in new_server_addr;
    memset(&new_server_addr, 0, sizeof(new_server_addr));
    new_server_addr.sin_family = AF_INET;
    new_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    new_server_addr.sin_port = htons(++UDP_TRANS_PORT);
    if (-1 == (bind(sockfd, (struct sockaddr *)&new_server_addr, sizeof(new_server_addr))))
    {
        perror("\tbind error\n");
        return ;
    }

    /* ʹ���½�����sockfd��ͻ��˷�����Ϣ */
    char new_ser_msg[BUFFSIZE] = "A new udp server for client";

    int se;
    se = sendto(sockfd, new_ser_msg, strlen(new_ser_msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    if (-1 == se)
    {
        perror("sendto error");
        close(sockfd);
        return ;
    }

    /* �ȴ�����ͻ��˻ش�����Ϣ */
    char tmp_msg[BUFFSIZE] = {0};
    int addrSize = sizeof(client_addr);
    while (1)
    {
        se = recvfrom(sockfd, tmp_msg, BUFFSIZE, 0, (struct sockaddr *)&client_addr, &addrSize);
        if (-1 == se)
        {
            perror("recvfrom error");
            close(sockfd);
            return ;
        }
        handle_udp_recv_msg(sockfd, client_addr, tmp_msg);
    }
}

/**
 * @brief ������Ϣ���Ͷ��յ���UDP��Ϣ���д���
 *        �ָ�str: �ϴ���Ϣ msg_list--Msg_Type, filename, filesize, hashcode
 *                ������Ϣ msg_list--Msg_Type, filename
 * @param sockfd, sockaddr_in--�ͻ��˵�ַ, str--��Ϣ�ִ�
 * @return void
 * */
void handle_udp_recv_msg(int sockfd, struct sockaddr_in client_addr, char* str)
{
    int i = 0;
    // char seg[] = ",";
    char msg_list[4][BUFFSIZE] = {0};
    char *substr = strtok(str, ",");    /* �á������ָ��ַ���str */
    while (NULL != substr)
    {
        strcpy(msg_list[i++], substr);
        substr = strtok(NULL, ",");
    }

    /* ѡ����� */
    switch (atoi(msg_list[0]))
    {
        case MSG_UDP_DOWNLOAD :
            udp_download_func(sockfd, client_addr, msg_list);
            break;
        case MSG_UDP_UPLOAD :
            udp_upload_func(sockfd, client_addr, msg_list);
            break;
        default :
            break;
    }
}

/**
 * @brief ����UDPģʽ�ļ����ز���
 * @param sockfd--tcp�����׽���������, sockaddr_in--�ͻ��˵�ַ, msg[][]--���յ�����Ϣ
 * @return void
 * */
void udp_download_func(int sockfd, struct sockaddr_in client_addr, char msg[][BUFFSIZE])
{
    char save_file[BUFFSIZE] = {0}; /* �����ļ�ʵ�ʴ��λ�� */
    sprintf(save_file, "%s%s", ser_place_of_file, msg[1]);

    char respon_msg[BUFFSIZE] = {0};
    strcpy(respon_msg, "0,0,0");    /* Ĭ��ֵ���ļ������ڣ� */
    if ((-1 == access(save_file, F_OK)))
    {
        sendto(sockfd, respon_msg, BUFFSIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    }
    else
    {
        struct stat s_buf;
        stat(save_file, &s_buf);    /* ��ȡ�ļ���Ϣ */
        if (S_ISDIR(s_buf.st_mode))
        {
            /* dir */
            sendto(sockfd, respon_msg, BUFFSIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            return ;
        }

        FILE *fp;
        char hashCode[41];  /* �������Ĺ�ϣֵ */
        /* �����䣬����ļ�У�麯�� */
        strcpy(hashCode, MY_FAKE_HASHCODE);

        memset(respon_msg, 0, sizeof(respon_msg));
        sprintf(respon_msg, "%d,%s,%d", 1, hashCode, get_file_size(save_file));
        sendto(sockfd, respon_msg, BUFFSIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

        fp = fopen(save_file, "rb");
        char buf[BUFFSIZE + 1] = {0};
        
        int readSize = 0;   /* ��ȡ���� */
        int sendSize = 0;   /* ���ͳ��� */
        int totalTrans = 0; /* ��¼�Ѿ������˶��� */
        int addrSize = sizeof(client_addr);
        while (1)
        {
            readSize = fread(buf, 1, BUFFSIZE, fp);
            if (0 == readSize)
            {
                break;
            }
            sendSize = sendto(sockfd, (const char *)buf, readSize, MSG_NOSIGNAL, (struct sockaddr *)&client_addr, sizeof(client_addr));

            recvfrom(sockfd, (char *)buf, BUFFSIZE, 0, (struct sockaddr *)&client_addr, &addrSize);
            totalTrans += sendSize;
        }

        /* ����ļ��������񣬷�����ʾ��Ϣ */
        sendto(sockfd, "OK", 2, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        printf("Complete send! Total trans %d bytes\n", totalTrans);
        fclose(fp);
    }
    
}

/**
 * @brief ����UDPģʽ�ļ��ϴ�����
 * @param sockfd--tcp�����׽���������, sockaddr_in--�ͻ��˵�ַ, msg[][]--���յ�����Ϣ
 * @return void
 * */
void udp_upload_func(int sockfd, struct sockaddr_in client_addr, char msg[][BUFFSIZE])
{
    char tmp_response[BUFFSIZE] = "udp response";

    FILE *fp;
    char save_file[BUFFSIZE];   /* �洢·��+�ļ��� */
    sprintf(save_file, "%s%s", ser_place_of_file, msg[1]);
    fp = fopen(save_file, "wb+"); /* �����ƴ� ����Դ�ļ� */
    int totalBytes = 0; /* ����ͳ�����ֽ��� */
    int addrSize = sizeof(client_addr);

    while (1)
    {
        char buf[BUFFSIZE] = {0};
        int len = recvfrom(sockfd, (char *)buf, BUFFSIZE, 0, (struct sockaddr *)&client_addr, &addrSize);

        /* �ͻ��˴������ */
        if (0 == strcmp(buf, "OK"))
        {
            if (totalBytes == atoi(msg[2]))
            {
                /* д���ֽ�����ͬ�������ܴ��ڰ����۸ĵ������У��hashcode */
                fclose(fp);
                char hashCode[41];
                char respons_msg[BUFFSIZE];
                /* �ļ�У��ģ���ʵ�� */
                // strcpy(hashCode, get_sha1_from_file(save_file)); /* ���ؼ���д����ɵ��ļ���hashcode */
                strcpy(hashCode, MY_FAKE_HASHCODE);
                if (0 == strcmp(hashCode, msg[3]))
                {
                    sprintf(respons_msg, "File Complete True; Sha1: %s", hashCode);       
                }
                else
                {
                    sprintf(respons_msg, "File Complete False; Sha1: %s", hashCode);
                }
                sendto(sockfd, respons_msg, BUFFSIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                break;
            }
        }

        fwrite(buf, 1, len, fp);
        /* д��ɹ�������Ӧ������ͻ��� */
        sendto(sockfd, tmp_response, BUFFSIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        totalBytes += len;
    }
}