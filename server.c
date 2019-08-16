/**
 * @file    server.c
 * @brief   服务端函数实现 
 * @author  lizidong
 * @date    2019/8/6
 * */
#include "server.h"
#include "tools.h"

char ser_place_of_file[BUFFSIZE];   /* 存放服务端文件存储路径 */

int main(int argc, char **argv)
{
    /* 默认在源文件同目录下创建serverFile文件夹 */
    strcpy(ser_place_of_file, "./serverFile/");
    mk_folder_of_file(ser_place_of_file);

    /* UDP组播监听线程 */
    pthread_t mulcast_thread;
    pthread_create(&mulcast_thread, NULL, (void *)respon_mulcast_init, NULL);

    /* TCP传输端口监听线程 */
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

    /* 主线程 */
    char buf[BUFFSIZE];
    while (1)
    {
        scanf("%s", buf);
    }
    // return 0;
}


/**
 * @brief 服务器端加入组播，响应组播信息
 * @param void
 * @return socket套接字标识符
 * */
int respon_mulcast_init()
{
    /* 新建socket 用于接收UDP组播消息，响应服务器探测 */
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        perror("fail to create socket(udp_respon_detect)");
        return ERROR;
    }
    
    /* 初始化地址为0 */
    struct sockaddr_in my_mulcast_addr;
    memset(&my_mulcast_addr, 0, sizeof(my_mulcast_addr));   /* 以0填充 */
    my_mulcast_addr.sin_family = AF_INET;
    my_mulcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_mulcast_addr.sin_port = htons(UDP_MCAST_PORT);

    /* 绑定套接字和端口 */
    if (-1 == bind(sockfd, (struct sockaddr *)&my_mulcast_addr, sizeof(my_mulcast_addr)))
    {
        perror("fail to bind (udp_mulcast)");
        return ERROR;
    }

    /* 初始化组播结构体变量 */
    struct ip_mreq mreq;
    memset(&mreq, 0x00, sizeof(struct ip_mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(UDP_MCAST_ADDR);  /* 设置组播地址 */
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);  /* 设置网络接口地址信息 */
    
    /* 加入组播组 */
    if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)))
    {
        perror("fail to add membership (join the mulcast)");
        return ERROR;
    }

    /* 禁止数据回传到本地环回接口 */
    unsigned char loop = 0;
    if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)))
    {
        perror("Error: set data loop");
        return ERROR;
    }


    struct sockaddr_in cli_addr;    /* 保存客户端信息 */
    char recv_msg[BUFFSIZE + 1];    /* 接收消息缓存 */
    unsigned int recv_size = 0;     /* 实际收到的消息大小 */
    unsigned int addr_size = sizeof(struct sockaddr_in);
    /* 循环接收组播消息 */
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
            recv_msg[recv_size] = '\0'; /* 成功接收数据报，末尾加结束符 */

            /* 获取ip和port */
            char ip_str[INET_ADDRSTRLEN];   /* IPv4地址，默认为16 */
            inet_ntop(AF_INET, &(cli_addr.sin_addr), ip_str, sizeof(ip_str));
            int port = ntohs(cli_addr.sin_port);
            
            /* 打印输出已收到探测信息 */
            printf("Received from %s--%d : [%s]\n", ip_str, port, recv_msg);
            /* 向客户端回复数据，单播 */
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
 * @brief 服务端TCP模式初始化，建立socket连接
 * @param void
 * @return socket套接字标识符
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

    int backlog = 10000;    /* 维护listen队列大小为10000 */
    if (-1 == (listen(sockfd, backlog))) 
    {
        perror("\tfail to listen");
        return ERROR;
    }

    // printf("succeed to init tcp socket!\n");
    return sockfd;
}

/**
 * @brief 服务端UDP连接初始化，建立socket连接
 * @param void
 * @return socket套接字标识符sockfd
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
    /* 绑定端口 */
    if (-1 == bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        perror("\tfail to bind udp socket");
        return ERROR;
    }

    return sockfd;
}

/**
 * @brief 监听TCP传输端口，等待客户端TCP连接
 *         成功accept一个tcp连接后，创建一个线程去处理
 * @param server端TCP传输sockfd
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

        /* 成功accept一个tcp客户端，为其创建一个处理消息线程 */
        pthread_t tcp_recv_thread;
        pthread_create(&tcp_recv_thread, NULL, (void *)deal_recv_tcp, &cli_sockfd);
        /* 设置分离态，使每个线程退出后自动释放资源 */
        pthread_detach(tcp_recv_thread);
    }
}

/**
 * @brief TCP连接处理线程，acceptTCP客户端后创建
 * @param 客户端TCP传输sockfd
 * @return void
 * */
void deal_recv_tcp(void* param)
{
    int cli_sockfd = *((int *)param);
    char buff[BUFFSIZE + 1] = {0};  /* 初始化缓存为0 */
    int recv_size;
    while (1)
    {
        recv_size = recv(cli_sockfd, buff, BUFFSIZE, 0);
        if (recv_size <= 0)
        {
            /* 关闭socket退出线程 已设置分离态，退出即释放资源 */
            close(cli_sockfd);  /* 避免过多socket处于close_wait状态 */
            pthread_exit(0);
            return ;
        }
        buff[recv_size] = '\0';
        handle_tcp_recv_msg(cli_sockfd, buff);
    }
}

/**
 * @brief 根据消息类型对收到的TCP消息进行处理
 * @param 客户端TCP传输sockfd   message字符串
 * @return void
 * */
void handle_tcp_recv_msg(int sockfd, char* str)
{
    /* 上传指令 分割str ： msg_list存放数据：msg_type, filename, filesize, hashcode */
    /* 下载指令 分割str ： msg_list存放数据：msg_type, filename, fileset, hashcode */
    int i = 0;
    // char seg[] = ",";
    char msg_list[4][BUFFSIZE] = {0};
    char *substr = strtok(str, ",");    /* 用“，”分割字符串str */
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
 * @brief 处理收到的ls命令,TCP传输
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
 * @brief 处理TCP模式文件下载操作
 *        respon_msg--"flag, hash, size"
 * @param sockfd--tcp连接套接字描述符 msg[][]--接收到的消息
 * @return void
 * */
void tcp_download_func(int sockfd, char msg[][BUFFSIZE])
{
    int file_set = atoi(msg[2]);    /* 从该位置继续发送,断点 */
    char save_file[BUFFSIZE] = {0}; /* 文件存放位置 */
    sprintf(save_file, "%s%s", ser_place_of_file, msg[1]);
    char respon_msg[BUFFSIZE] = {0};
    strcpy(respon_msg, "0,0,0");    /* 默认值（文件不存在） */
    /* 文件不存在 */
    if (-1 == access(save_file, F_OK))
    {
        send(sockfd, respon_msg, BUFFSIZE, 0);
    }
    else
    {
        struct stat s_buf;
        stat(save_file, &s_buf);    /* 获取资源信息 */
        if (S_ISDIR(s_buf.st_mode))
        {
            /* 目录 */
            send(sockfd, respon_msg, BUFFSIZE, 0);
            return ;
        }

        FILE *fp;
        char hashCode[41];  /* 保存计算的哈希值 */
        /* 待扩充，添加文件校验函数 */
        strcpy(hashCode, MY_FAKE_HASHCODE);
        /* msg[3]--hashcode, 需要续传 */
        if (0 == strcmp(hashCode, msg[3]))
        {
            sprintf(respon_msg, "%d,%s,%d", 2, hashCode, get_file_size(save_file));
            send(sockfd, respon_msg, BUFFSIZE, 0);
        }
        /* 新下载任务，不需要续传 */
        else 
        {
            sprintf(respon_msg, "%d,%s,%d", 1, hashCode, get_file_size(save_file));
            send(sockfd, respon_msg, BUFFSIZE, 0);
            file_set = 0;   /* 针对覆盖的情况 */
        }

        fp = fopen(save_file, "rb");
        char buf[BUFFSIZE + 1] = {0};
        fseek(fp, file_set, SEEK_SET);  /* 定位到客户端上传的偏移位置 */
        int readSize = 0;   /* 读取长度 */
        int sendSize = 0;   /* 发送长度 */
        int totalTrans = 0; /* 记录已经发送了多少 */
        while (1)
        {
            readSize = fread(buf, 1, BUFFSIZE, fp);
            if (0 == readSize)
            {
                break;
            }
            /* 客户端断开，服务端向一个失效的socket发送数据时，底层抛出SIGIPE信号
            * 该信号缺省处理方式为退出服务器进程，设置MSG_NOSIGNAL忽略该信号
            * */
            sendSize = send(sockfd, (const char *)buf, readSize, MSG_NOSIGNAL);
            if (-1 == sendSize)
            {
                /* 发送出错，客户端异常断开 */
                /* 断点续传功能在数据接收端实现 */
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
 * @brief 处理TCP模式文件上传操作
 * @param sockfd--tcp连接套接字描述符 msg[][]--接收到的消息
 * @return void
 * */
void tcp_upload_func(int sockfd, char msg[][BUFFSIZE])
{
    int flag = 0;   /* 默认无需续传 */
    int bp_size = 0;    /* 用于fseek定位断点 */

    /* 查找是否存在断点文件 */
    char bp_file[BUFFSIZE];
    sprintf(bp_file, "%s.%s.bot.break", ser_place_of_file, msg[1]);
    char tmp_zero[BUFFSIZE];    /* 用于向客户端发送 */
    strcpy(tmp_zero, "0");
    if (access(bp_file, F_OK) != -1)
    {
        /* 文件存在 */
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
            /* 文件名和哈希值相同，续传 */
            flag = 1;
            bp_size = atoi(tmp_size);
            send(sockfd, tmp_size, BUFFSIZE, 0);
            remove(bp_file);
        }
        else
        {
            /* 同名，直接覆盖 */
            send(sockfd, tmp_zero, BUFFSIZE, 0);
        }
    }
    else
    {
        /* 文件不存在 */
        send(sockfd, tmp_zero, BUFFSIZE, 0);
    }

    FILE *fp;
    char save_file[BUFFSIZE];   /* 存储路径+文件名 */
    sprintf(save_file, "%s%s", ser_place_of_file, msg[1]);
    if (0 == flag)   
    {
        /* 不需要续传 */
        fp = fopen(save_file, "wb+");    /* 二进制打开， 擦除源文件 */
    }
    else
    {
        fp = fopen(save_file, "rb+");   /* 二进制打开， 不擦除源文件 */
    }
    
    int totalBytes = 0; /* 用于统计总字节数 */
    fseek(fp, bp_size, SEEK_SET);   /* 跳转到指定位置（断点） */
    while (1)
    {
        char buf[BUFFSIZE] = {0};
        int len = recv(sockfd, (char *)buf, BUFFSIZE, 0);
        if (0 == len)
        {
            /* 客户端异常断开 */
            fclose(fp);

            /* 创建断点文件， 格式为.filename.bot.break 隐藏，对用户透明 */
            FILE *fp_break;
            char break_file[BUFFSIZE];
            sprintf(break_file, "%s.%s.bot.break", ser_place_of_file, msg[1]);
            /* 对同名文件的断点续传，只处理后面的续传，前面的被后面的同名文件覆盖 */
            fp_break = fopen(break_file, "w+");
            char str_totalBytes[BUFFSIZE];
            sprintf(str_totalBytes, "%d", totalBytes);
            fwrite(str_totalBytes, 1, strlen(str_totalBytes), fp_break);    /* 写入已接收长度 */
            fwrite("\r\n", 1, 2, fp_break); /* 换行 */
            fwrite(msg[3], 1, strlen(msg[3]), fp_break);  /* 写入hashcode */
            fclose(fp_break);
            break;
        }

        fwrite(buf, 1, len, fp);

        totalBytes += len;
        /* 写入字节数相同，但可能存在包被篡改的情况，需校验hashcode */
        if (totalBytes == (atoi(msg[2]) - bp_size))
        {
            fclose(fp);
            char hashCode[41];
            char respons_msg[BUFFSIZE];
            /* 文件校验模块待实现 */
            // strcpy(hashCode, get_sha1_from_file(save_file)); /* 本地计算写入完成的文件的hashcode */
            strcpy(hashCode, MY_FAKE_HASHCODE);
            
            if (0 == strcmp(hashCode, msg[3]))
            {
                /* 非续传返回 */
                if (0 == flag)   
                {
                    sprintf(respons_msg, "File Complete True; Sha1: %s", hashCode);
                }
                /* 续传返回 */
                else
                {
                    sprintf(respons_msg, "Continue File Complete True; Sha1: %s", hashCode);
                }
                send(sockfd, respons_msg, BUFFSIZE, 0);
            }
            else
            {
                /* 非续传返回 */
                if (0 == flag)   
                {
                    sprintf(respons_msg, "File Complete False; Sha1: %s", hashCode);
                }
                /* 续传返回 */
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
 * @brief 监听UDP传输端口，等待客户端UDP连接
 *         为每个客户创建一个线程去处理，每个客户对应一个端口
 * @param server端UDP传输sockfd
 * @return void
 * */
void wait_udp_connect(void* param)
{
    int serv_sockfd = *((int *)param);
    while (1)
    {
        char tmp_msg[BUFFSIZE] = {0};   /* 存放来自客户端的消息，不做其他处理 */

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
        
        tmp_client_addr = malloc(sizeof(struct sockaddr_in));   /* 线程结构体需要分配到堆上 */
        *tmp_client_addr = client_addr;

        /* 设置分离态，使每个线程在退出后自动释放资源 */
        pthread_t udp_recv_thread;
        pthread_create(&udp_recv_thread, NULL, (void *)deal_recv_udp, tmp_client_addr);
        pthread_detach(udp_recv_thread);
    }
}

/**
 * @brief 为每个udp客户端创建一个处理线程
 * @param 该UDP客户端的sockaddr_in结构体
 * @return void
 * */
void deal_recv_udp(void* param)
{
    struct sockaddr_in client_addr = *((struct sockaddr_in *)param);
    free(param);

    /* 创建新的sockfd, 为每个UDP客户端分配新的端口 */
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

    /* 使用新建立的sockfd向客户端发送消息 */
    char new_ser_msg[BUFFSIZE] = "A new udp server for client";

    int se;
    se = sendto(sockfd, new_ser_msg, strlen(new_ser_msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    if (-1 == se)
    {
        perror("sendto error");
        close(sockfd);
        return ;
    }

    /* 等待处理客户端回传的消息 */
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
 * @brief 根据消息类型对收到的UDP消息进行处理
 *        分割str: 上传消息 msg_list--Msg_Type, filename, filesize, hashcode
 *                下载消息 msg_list--Msg_Type, filename
 * @param sockfd, sockaddr_in--客户端地址, str--消息字串
 * @return void
 * */
void handle_udp_recv_msg(int sockfd, struct sockaddr_in client_addr, char* str)
{
    int i = 0;
    // char seg[] = ",";
    char msg_list[4][BUFFSIZE] = {0};
    char *substr = strtok(str, ",");    /* 用“，”分割字符串str */
    while (NULL != substr)
    {
        strcpy(msg_list[i++], substr);
        substr = strtok(NULL, ",");
    }

    /* 选择操作 */
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
 * @brief 处理UDP模式文件下载操作
 * @param sockfd--tcp连接套接字描述符, sockaddr_in--客户端地址, msg[][]--接收到的消息
 * @return void
 * */
void udp_download_func(int sockfd, struct sockaddr_in client_addr, char msg[][BUFFSIZE])
{
    char save_file[BUFFSIZE] = {0}; /* 保存文件实际存放位置 */
    sprintf(save_file, "%s%s", ser_place_of_file, msg[1]);

    char respon_msg[BUFFSIZE] = {0};
    strcpy(respon_msg, "0,0,0");    /* 默认值（文件不存在） */
    if ((-1 == access(save_file, F_OK)))
    {
        sendto(sockfd, respon_msg, BUFFSIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    }
    else
    {
        struct stat s_buf;
        stat(save_file, &s_buf);    /* 获取文件信息 */
        if (S_ISDIR(s_buf.st_mode))
        {
            /* dir */
            sendto(sockfd, respon_msg, BUFFSIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            return ;
        }

        FILE *fp;
        char hashCode[41];  /* 保存计算的哈希值 */
        /* 待扩充，添加文件校验函数 */
        strcpy(hashCode, MY_FAKE_HASHCODE);

        memset(respon_msg, 0, sizeof(respon_msg));
        sprintf(respon_msg, "%d,%s,%d", 1, hashCode, get_file_size(save_file));
        sendto(sockfd, respon_msg, BUFFSIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

        fp = fopen(save_file, "rb");
        char buf[BUFFSIZE + 1] = {0};
        
        int readSize = 0;   /* 读取长度 */
        int sendSize = 0;   /* 发送长度 */
        int totalTrans = 0; /* 记录已经发送了多少 */
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

        /* 完成文件传输任务，发送提示消息 */
        sendto(sockfd, "OK", 2, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        printf("Complete send! Total trans %d bytes\n", totalTrans);
        fclose(fp);
    }
    
}

/**
 * @brief 处理UDP模式文件上传操作
 * @param sockfd--tcp连接套接字描述符, sockaddr_in--客户端地址, msg[][]--接收到的消息
 * @return void
 * */
void udp_upload_func(int sockfd, struct sockaddr_in client_addr, char msg[][BUFFSIZE])
{
    char tmp_response[BUFFSIZE] = "udp response";

    FILE *fp;
    char save_file[BUFFSIZE];   /* 存储路径+文件名 */
    sprintf(save_file, "%s%s", ser_place_of_file, msg[1]);
    fp = fopen(save_file, "wb+"); /* 二进制打开 擦除源文件 */
    int totalBytes = 0; /* 用于统计总字节数 */
    int addrSize = sizeof(client_addr);

    while (1)
    {
        char buf[BUFFSIZE] = {0};
        int len = recvfrom(sockfd, (char *)buf, BUFFSIZE, 0, (struct sockaddr *)&client_addr, &addrSize);

        /* 客户端传输完成 */
        if (0 == strcmp(buf, "OK"))
        {
            if (totalBytes == atoi(msg[2]))
            {
                /* 写入字节数相同，但可能存在包被篡改的情况，校验hashcode */
                fclose(fp);
                char hashCode[41];
                char respons_msg[BUFFSIZE];
                /* 文件校验模块待实现 */
                // strcpy(hashCode, get_sha1_from_file(save_file)); /* 本地计算写入完成的文件的hashcode */
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
        /* 写入成功，发送应打包给客户端 */
        sendto(sockfd, tmp_response, BUFFSIZE, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        totalBytes += len;
    }
}