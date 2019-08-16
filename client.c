/**
 * @file    client.c
 * @brief   客户端实现
 * @author  lizidong
 * @date    2019/8/6
 * */
#include "client.h"
#include "tools.h"

int cli_tcp_sockfd = -1;                /* 全局 tcp socket 初始化负值不可用 */    
int cli_udp_sockfd = -1;                /* 全局 udp socket 初始化负值不可用 */

char *SERVER_ADDR;                      /* 全局服务端IP地址 */
char cli_place_of_file[BUFFSIZE];       /* 全局定义客户端文件存放位置 */
struct sockaddr_in cli_udp_sockaddr;    /* 获取全局UDP新服务端处理端口 */

int main(int argc, char *argv)
{
    /* 在同目录下创建一个clientFile文件夹存放客户端文件 */
    strcpy(cli_place_of_file, "./clientFile/");
    mk_folder_of_file(cli_place_of_file);

    menu();     /* 初始化打印菜单 */
    // int op;
    char input[BUFFSIZE];   /* 屏幕输入缓存 */
    while (1)
    {
        hightlight_Command();
        fgets(input, BUFFSIZE, stdin);  /* 从标准输入获取命令 */
        if (input[strlen(input) - 1] == '\n')
        {
            input[strlen(input) - 1] = '\0';
        }

        switch (handle_input(input))
        {
            case Sys_Help :
                menu();
                break;
            case Sys_Detect :
                detectServer();
                break;
            case Sys_Connect :
                connect_client(cmd);
                break;
            case Sys_Ls_Client :
                ls_file_client();
                break;
            case Sys_Ls_Server :
                ls_file_server();
                break;
            case Sys_Download :
                interface_download_client(cmd);
                break;
            case Sys_Upload :
                interface_upload_client(cmd);
                // my_interface_upload_client(cmd);
                break;
            case Sys_Quit :
                goto _exit_system;
                break;
            case ERROR_NO_INPUT :
                printf("Please Enter At Least One Command!\r\n");
                break;
            case ERROR_INVALID_INPUT :
                printf("Wrong Command! Please Input again!\n");
                break;
            default :
                break;
        }
    }
    
_exit_system:
    close(cli_tcp_sockfd);
    close(cli_udp_sockfd);
    printf("Successful Exit the System!\n");
    return 0;
}

/**
 * @brief 操作指示菜单
 * @param 无
 * @return 无
 * */
void menu()
{
    printf("**************************************************************************\n");
    printf("**\t\tPlease enter the command of your choice\t\t\t**\n");
    printf("**----------------------------------------------------------------------**\n");
    printf("**  help       --  show the helpmenu\t\t\t\t\t**\n");
    printf("**  detect     --  scan lan network, detect online server\t\t**\n");
    printf("**  connect    --  connect to a server;\t\t\t\t\t**\n");
    printf("**                 Enter command as: 'connect ipaddress'\t\t**\n");
    printf("**  ls_client  --  list available files on the client\t\t\t**\n");
    printf("**  ls_server  --  list available files on the server\t\t\t**\n");
    printf("**  download   --  download from server; \t\t\t\t**\n");
    printf("**                 Enter command as: 'download tcp/udp filename'\t**\n");
    printf("**  upload     --  upload to server; \t\t\t\t\t**\n");
    printf("**                 Enter command as: 'upload tcp/udp filename'\t\t**\n");
    printf("**  quit       --  exit the system\t\t\t\t\t**\n");
    printf("**************************************************************************\n");
}

/**
 * @brief 处理输入，解析命令
 * @param 输入字符串
 * @return 返回选择操作符
 * */
int handle_input(char* str)
{
    int i;
    for (i = 0; i < 3; i++) 
    {
        cmd[i][0] = '\0';   /* 初始化为空 */
    }
    sscanf(str, "%s %s %s", cmd[0], cmd[1], cmd[2]);
    for (i = 0; i < MAX_CMD_NUM; i++)
    {
        /* 忽略大小写比对命令 */
        if (0 == strcasecmp(cmd[0], SysCommand[i]))
        {
            return i;
        }
    }
    if (0 == strcasecmp(cmd[0], ""))
    {
        return ERROR_NO_INPUT;  /* 无输入 */
    }
    return ERROR_INVALID_INPUT; /* 非法输入 */
}

/*------------------------------------------服务器探测模块---------------------------------------- */
/**
 * @brief 服务器探测模块,UDP组播方式探测服务器
 * @param 无
 * @return 正常退出返回0, 异常退出返回-1
 * */
int detectServer()
{
    printf("-----Quit the detect module : Just enter 'q'-------\n");

    int sockfd; /* 定义套接字，函数结束即关闭*/
    if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        perror("fail to create socket");
        return ERROR;
    }

    struct sockaddr_in mulcast_addr;  /* 定义多播地址结构 */
    memset(&mulcast_addr, 0, sizeof(mulcast_addr)); /* 初始化多播地址为0 */
    mulcast_addr.sin_family = AF_INET;  /* 设置协议族为IPv4 */
    mulcast_addr.sin_addr.s_addr = inet_addr(MCAST_ADDR);   /* 设置多播IP地址 */
    mulcast_addr.sin_port = htons(MCAST_PORT);  /* 设置多播端口 */

    /* 组播发送数据 */
    int size_req = sendto(sockfd, MCAST_DATA, sizeof(MCAST_DATA), 0, 
        (struct sockaddr *)&mulcast_addr, sizeof(mulcast_addr));
    if (size_req < 0)
    {
        perror("fail to send detect message");
        return ERROR;
    }

    /* 发送组播消息后，循环接受服务端9878端口发来的消息，有消息说明服务器在线 */
    pthread_t mulcast_msg_thread;
    pthread_create(&mulcast_msg_thread, NULL, (void *)print_detect_result, &sockfd);
    pthread_detach(mulcast_msg_thread); /* 关闭线程，释放资源 */

    /* recvfrom() 和 getchar() 均采用阻塞模式，另开线程用于接受显示 */
    while (1)
    {
        if (getch() == 'q')
        {
            pthread_cancel(mulcast_msg_thread); /* 取消线程信号 */
            return 0;
        }
    }
}

/**
 * @brief 打印输出UDP探测结果，位于子线程中
 * @param void*
 * @return void
 * */
void print_detect_result(void* param)
{
    int sockfd = *((int *)param);
    struct sockaddr_in online_server_addr;  /* 记录可用服务器信息 */
    
    char recv_msg[BUFFSIZE + 1];    /* 消息接收buff */
    unsigned int recv_size = 0;     /* 接收buff实际占用大小，初始化为0 */
    unsigned int addr_size = sizeof(struct sockaddr_in);

    /* 设置立即取消 recvfrom */
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (1)
    {
        memset(recv_msg, 0x00, BUFFSIZE + 1);
        recv_size = recvfrom(sockfd, recv_msg, BUFFSIZE, 0, (struct sockaddr *)&online_server_addr, &addr_size);
        if (recv_size < 0 || 0 == recv_size)
        {
            perror("recvfrom error");
            pthread_exit(0);
            return ;
        }
        else 
        {
            /* 成功接收到数据报，末尾加结束符'\0' */
            recv_msg[recv_size] = '\0';

            /* 提取IP地址和端口号 */
            char ip_str[INET_ADDRSTRLEN];   /* IPv4地址 */
            inet_ntop(AF_INET, &(online_server_addr.sin_addr), ip_str, sizeof(ip_str));
            int port = ntohs(online_server_addr.sin_port);

            /* 更新全局IP地址为探测到的IP */
            // SERVER_ADDR = ip_str;

            // printf("Server IP--%s\tPort--%d\t\n", ip_str, port);
            // printf("Received Msg--%s\n", recv_msg);
            printf("server ip -- port : [received msg]\n");
            printf("%s--%d : [%s]\n", ip_str, port, recvmsg);
        }
    }

}


/*-------------------------------以下为初始化建立connect模块实现-------------------------------*/
/**
 * @brief 客户端connect模块接口
 * @param ip_addr--服务器地址
 * @return 返回socket套接字描述符
 * */
void connect_client(char cmd_str[][64])
{
    if (0 == strcasecmp(cmd_str[1], ""))
    {
        /* 缺少ip地址参数 */
        printf("\tPlease Enter one command like this : 'connect IpAddress'\r\n");
    }
    // else if (0 == strcasecmp(cmd_str[1], SERVER_ADDR))
    else 
    {
        /* 输入IP合法，是探测到的IP地址，继续接下来的操作 */
        /* tcp mode */
        close(cli_tcp_sockfd);  /* 关闭已有tcp socket */
        if ((cli_tcp_sockfd = init_tcp_client(cmd_str[1])) < 0)
        {
            /* 初始化错误，无法连接 */
            printf("\tCan't create TCP conncet! Check if server online!\r\n");
        }
        else
        {
            printf("Success to establish a new TCP connection!\n");
        }

        /* udp mode */
        close(cli_udp_sockfd);  /* 关闭已有udp socket */
        memset(&cli_udp_sockaddr, 0, sizeof(cli_udp_sockaddr));
        if ((cli_udp_sockfd = init_udp_client(cmd_str[1])) < 0)
        {
            /* 初始化错误，无法连接 */
            printf("\tFail to get UDP server! Check if server online!\r\n");   
        }
        else 
        {
            printf("Success to get UDP server! \n");
        }
    }
    
}
/**
 * @brief 客户端初始化建立TCP连接
 * @param ip_addr--服务器地址
 * @return 返回socket套接字描述符
 * */
int init_tcp_client(char *ip_addr)
{
    int sockfd; //套接字描述符
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;   /*IPv4连接 */
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);  /*服务器地址 */
    server_addr.sin_port = htons(TCP_TRANS_PORT); /*TCP传输端口 */

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

    // printf("success to create TCP connect!\n");

    return sockfd;
}

/**
 * @brief 客户端初始化建立UDP连接
 * @param ip_addr--服务器地址
 * @return 返回socket套接字描述符
 * */
int init_udp_client(char *ip_addr)
{
    int sockfd;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    int addr_size = sizeof(struct sockaddr_in);
    char first_msg[BUFFSIZE] = "udp client requst to connect";

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(UDP_TRANS_PORT);

    if (-1 == (sockfd = socket(AF_INET, SOCK_DGRAM, 0)))
    {
        perror("fail to create UDP socket");
        return ERROR;
    }
    
    sendto(sockfd, first_msg, strlen(first_msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    memset(first_msg, 0, sizeof(first_msg));    /* 重置first_msg */
    /* 获取新的服务端端口信息 */
    int new_size = recvfrom(sockfd, first_msg, BUFFSIZE, 0, (struct sockaddr *)&server_addr, &addr_size);
    if (-1 == new_size)
    {
        return ERROR;
    }

    /* 成功获取新的服务端端口地址信息，用于进行下一步上传或下载操作 */
    /* 更新全局udp_sockaddr */
    cli_udp_sockaddr = server_addr;
    
    return sockfd;
}

/*-------------------------------以下为获取本地文件列表ls_client模块实现-------------------------------*/

/**
 * @brief 获取客户端可下载文件列表
 * @param void
 * @return void
 * */
void ls_file_client()
{
    DIR *dir;
    struct dirent *ptr;

    if (NULL == (dir = opendir(cli_place_of_file)))
    {
        /* 目录打开失败 */
        printf("No available file in client!\n");
        return ;
    }
    else
    {
        printf("file list in client as follow:\n\n");
        while ((ptr = readdir(dir)) != NULL)
        {
            /* current dir or parrent dir */
            if (strcmp(ptr->d_name, ".") == 0 ||  0 == strcmp(ptr->d_name, ".."))
            {
                continue;
            }
            else if (8 == ptr->d_type)
            {
                /* file */
                printf("%s\n", ptr->d_name);
            }
            else
            {
                continue;
            }
            
        }
    }
    closedir(dir);
    
}

/*-------------------------------以下为获取服务端文件列表ls_server模块实现-------------------------------*/
/**
 * @brief 获取服务端可下载文件列表
 * @param void
 * @return void
 * */
void ls_file_server()
{
    if (cli_tcp_sockfd < 0)
    {
        printf("\tNo available server! Please connect one first!\n");
    }
    else
    {
        char str[2 * BUFFSIZE] = {0};
        sprintf(str, "%d,0", MSG_TCP_LS);
        /* 发送ls命令给服务端， 格式为Msg_Type,0 */
        send(cli_tcp_sockfd, str, strlen(str), 0);
        /* 接收服务端传回的文件信息 */
        recv(cli_tcp_sockfd, str, 2 * BUFFSIZE, 0);
        printf("%s\n", str);
    }
    
}
/*-------------------------------以下为下载文件download模块实现-------------------------------*/
/**
 * @brief 客户端下载模块实现接口
 * @param cmd_str---屏幕输入指令
 * @return void
 * */
void interface_download_client(char cmd_str[][64])
{
    // char trans_mode[20] = cmd_str[1];
    // char filename[20] = cmd_str[2];
    char trans_mode[64];
    char filename[64];
    strcpy(trans_mode, cmd_str[1]);
    strcpy(filename, cmd_str[2]);
    /* filename为空 */
    if (0 == strcasecmp(filename, ""))
    {
        printf("\tWrong Command! Enter: 'upload TCP/UDP filename'\n");
    }
    else
    {
        /* tcp mode */
        if (0 == strcasecmp(trans_mode, "tcp"))
        {
            if (cli_tcp_sockfd < 0) 
            {
                /* socket不正常，尚未建立TCP连接 */
                printf("\tNo available server! Please connect one first!\n");
            }
            else
            {
                /* 添加udp模式下载文件处理函数 */
                client_download_tcp(filename);
            }
        }
        /* udp mode */
        else if (0 == strcasecmp(trans_mode, "udp"))
        {
            if (cli_udp_sockfd < 0)
            {
                /* 未连接udp服务器 */
                printf("\tPlease check if udp server online!\n");
            }
            else
            {
                /* 添加udp模式文件下载处理函数 */
                client_download_udp(filename);
            }
                    
        }
        /* 输入非法指令 */
        else
        {
            printf("\tWrong Command! Enter: 'upload TCP/UDP filename'\n");
        }    
    }    
}

/**
 * @brief TCP模式下从服务器下载文件
 *        socket使用全局sockfd，注意其是否已正确初始化
 *        接收消息格式为：
 *          "Msg_Type, FileName, FileSet, HashCode"
 *        FileSet   一般为0， 有断点文件时为size值
 *        HashCode  一般为0, 有断点文件时为hashcode
 *        断点文件结构：    size \r\n hashcode
 * @param filename--文件名,不包括路径
 * @return 正常下载返回0, 异常返回ERROR
 * */
int client_download_tcp(char* filename)
{
    /* 判断请求下载文件输入格式是否合法 */
    if ((strchr(filename, '/') != NULL) || (!strcmp(filename, ".")) || (!strcmp(filename, "..")))
    {
        printf("Wrong filename! Please enter format filename!\n");
        return 0;
    }

    char tmp_size[BUFFSIZE] = {0};
    strcpy(tmp_size, "0");
    char tmp_hashcode[BUFFSIZE] = {0};
    strcpy(tmp_hashcode, "0");

    char save_file[BUFFSIZE] = {0};
    sprintf(save_file, "%s%s", cli_place_of_file, filename);
    
    /* 检索断点文件 */
    char bp_file[BUFFSIZE];
    sprintf(bp_file, "%s.%s.bot.break", cli_place_of_file, filename);
    /* 找到断点文件 */
    if (access(bp_file, F_OK) != -1)
    {
        FILE *fp_bp;
        fp_bp = fopen(bp_file, "r");
        fgets(tmp_size, BUFFSIZE, fp_bp);
        tmp_size[strlen(tmp_size) - 1] = '\0';
        fgets(tmp_hashcode, BUFFSIZE, fp_bp);
        fclose(fp_bp);
    }

    char str[BUFFSIZE] = {0};
    sprintf(str, "%d,%s,%d,%s", MSG_TCP_DOWNLOAD, filename, atoi(tmp_size), tmp_hashcode);
    /* 向服务器发送下载请求，格式为 Msg_Type, filename, fileset, hashcode */
    send(cli_tcp_sockfd, str, strlen(str), 0);
    /* 接收服务端返回的文件基本信息， 格式为is_exist, hashcode, filesize */
    /* is_exist -- 0:文件不存在 1:非续传 2:续传 */
    recv(cli_tcp_sockfd, str, BUFFSIZE, 0);

    /* 处理收到的信息，提取信息，存放在my_msg里 */
    char my_msg[3][BUFFSIZE];
    /* my_msg[0]--is_exist, my_msg[1]--hashcode, my_msg[2]--filesize */
    int i = 0;
    char *substr = strtok(str, ",");
    while (substr != NULL)
    {
        strcpy(my_msg[i++], substr);
        substr = strtok(NULL, ",");
    }

    if (0 == atoi(my_msg[0]))   /* 文件不存在 */
    {
        /* 请求的文件不存在，不删除断点文件 */
        printf("\tThe file you requested has not found!\n");
        return 0;
    }
    /* 请求的文件存在，验证hashcode,相同则续传;不同则覆盖 删除断点文件 */
    else
    {
        /* 请求的文件存在并且有断点文件，删除 */
        if (0 != strcmp(tmp_hashcode, "0"))
        {
            remove(bp_file);
        }

        FILE *fp;
        /* 不需要续传 */
        if(1 == atoi(my_msg[0]))
        {
            fp = fopen(save_file, "wb+");   /* 二进制打开，擦除原文件 */
            strcpy(tmp_size, "0");  /* 重置为0,文件覆盖 */
        }
        /* 需要续传 */
        else
        {
            fp = fopen(save_file, "rb+");   /* 二进制打开，不擦除原文件 */
        }
        
        int totalBytes = 0; /* 用于统计下载总字节数 */
        fseek(fp, atoi(tmp_size), SEEK_SET);    /* 跳转到指定位置(断点) */
        while (1)
        {
            char buf[BUFFSIZE] = {0};
            int len = recv(cli_tcp_sockfd, (char *)buf, BUFFSIZE, 0);
            /* 服务端断开，生成断点文件 */
            if (0 == len)
            {
                fclose(fp);

                /* 创建断点文件， 格式为.filename.bot.break，隐藏，对用户透明 */
                FILE *fp_break;
                /* 对同名文件的断点续传，只处理后面的续传，前面的被后面的同名文件覆盖 */
                fp_break = fopen(bp_file, "w+");
                char str_totalBytes[BUFFSIZE];
                sprintf(str_totalBytes, "%d", totalBytes);
                fwrite(str_totalBytes, 1, strlen(str_totalBytes), fp_break);    /* 写入已接收长度 */
                fwrite("\r\n", 1, 2, fp_break);
                fwrite(my_msg[1], 1, strlen(my_msg[1]), fp_break);  /* 写入hashcode */
                fclose(fp_break);
                break;
            }

            fwrite(buf, 1, len, fp);
            totalBytes += len;
            double recvPer = 1.0 * totalBytes / (atoi(my_msg[2]) - atoi(tmp_size)) * 100;
            printf("Receiving %s...   %.02f%% completed\n", filename, recvPer);
            // printf("%d of %s has received!\n", totalBytes, atoi(my_msg[2]) - atoi(tmp_size));
            
            /* 写入字节数相同，但可能存在包被篡改的情况，需校验hashcode */
            if (totalBytes == (atoi(my_msg[2]) - atoi(tmp_size)))
            {
                fclose(fp);
                char hashCode[41];
                // strcpy(hashCode, get_sha1_from_file(save_file)); /* 本地计算写入完成的文件的hashcode */
                /* 文件校验模块待实现 */
                strcpy(hashCode, MY_FAKE_HASHCODE);
                
                if (0 == strcmp(hashCode, my_msg[1]))
                {
                    /* 非续传返回 */
                    if (1 == atoi(my_msg[0]))   
                    {
                        printf("Receive file True; Sha1: %s\n", hashCode);
                    }
                    /* 续传返回 */
                    else
                    {
                        printf("Continue receive file True; Sha1: %s\n", hashCode);
                    }
                    
                }
                else
                {
                    /* 非续传返回 */
                    if (1 == atoi(my_msg[0]))   
                    {
                        printf("Receive file False; Sha1: %s\n", hashCode);
                    }
                    /* 续传返回 */
                    else
                    {
                        printf("Continue receive file False; Sha1: %s\n", hashCode);
                    }
                }
                
                break;
            }
        }
        
    }
    return 0;
}

/**
 * @brief UDP模式下从服务器下载文件
 *        socket使用全局sockfd，注意其是否已正确初始化
 *        接收消息格式为：
 *          "Msg_Type, FileName"
 * @param filename--文件名,不包括路径
 * @return 正常下载返回0, 异常返回ERROR
 * */
int client_download_udp(char* filename)
{
    /* 判断请求下载文件输入格式是否合法 */
    if ((strchr(filename, '/') != NULL) || (!strcmp(filename, ".")) || (!strcmp(filename, "..")))
    {
        printf("Wrong filename! Please enter format filename!\n");
        return 0;
    }
    
    char save_file[BUFFSIZE] = {0};
    sprintf(save_file, "%s%s", cli_place_of_file, filename);

    char tmp_response[BUFFSIZE] = "response";
    char str[BUFFSIZE] = {0};
    int add_size = sizeof(cli_udp_sockaddr);
    sprintf(str, "%d,%s", MSG_UDP_DOWNLOAD, filename);

    /* 向服务器发送下载请求，格式为 Msg_Type, filename */
    sendto(cli_udp_sockfd, str, strlen(str), 0, (struct sockaddr *)&cli_udp_sockaddr, sizeof(cli_udp_sockaddr));
    /* 接收服务端返回的文件基本信息， 格式为is_exist, hashcode, filesize */
    /* is_exist -- 0:文件不存在 */
    recvfrom(cli_udp_sockfd, str, BUFFSIZE, 0, (struct sockaddr *)&cli_udp_sockaddr, &add_size);

    /* 处理收到的信息，提取信息，存放在my_msg里 */
    char my_msg[3][BUFFSIZE];
    /* my_msg[0]--is_exist, my_msg[1]--hashcode, my_msg[2]--filesize */
    int i = 0;
    char *substr = strtok(str, ",");
    while (substr != NULL)
    {
        strcpy(my_msg[i++], substr);
        substr = strtok(NULL, ",");
    }

    if (0 == atoi(my_msg[0]))   /* 文件不存在 */
    {
        /* 请求的文件不存在，不删除断点文件 */
        printf("\tThe file you requested has not found!\n");
        return 0;
    }
    /* 请求的文件存在 */
    else
    {
        FILE *fp;
        fp = fopen(save_file, "wb+");   /* 二进制打开，擦除源文件 */
                
        int totalBytes = 0; /* 用于统计下载总字节数 */
        while (1)
        {
            char buf[BUFFSIZE] = {0};
            int len = recvfrom(cli_udp_sockfd, (char *)buf, BUFFSIZE, 0, (struct sockaddr *)&cli_udp_sockaddr, &add_size);
            
            /* 服务端发送文件完成 */
            if (0 == strcmp(buf, "OK"))
            {
                /* 写入字节数相同，但可能存在包被篡改的情况，需校验hashcode */
                if (totalBytes == atoi(my_msg[2]))
                {
                    fclose(fp);
                    char hashCode[41];
                    // strcpy(hashCode, get_sha1_from_file(save_file)); /* 本地计算写入完成的文件的hashcode */
                    /* 文件校验模块待实现 */
                    strcpy(hashCode, MY_FAKE_HASHCODE);
                    if (0 == strcmp(hashCode, my_msg[1]))
                    {
                        printf("Receive file True; Sha1: %s\n", hashCode);
                    }
                    else
                    {
                        printf("Receive file False; Sha1: %s\n", hashCode);
                    }
                    break;  
                }
            }
            
            fwrite(buf, 1, len, fp);
            /* 完成写入，发送响应包 */
            sendto(cli_udp_sockfd, tmp_response, BUFFSIZE, 0, (struct sockaddr *)&cli_udp_sockaddr, sizeof(cli_udp_sockaddr));

            totalBytes += len;
            double recvPer = 1.0 * totalBytes / (atoi(my_msg[2])) * 100;
            printf("Receiving %s...   %.02f%% completed\n", filename, recvPer);
            // printf("%d of %d has received!\n", totalBytes, atoi(my_msg[2]));
                        
        }
        
    }
    return 0;
}


/*-------------------------------以下为上传文件upload模块实现-------------------------------*/
/**
 * @brief 客户端上传模块实现接口
 * @param cmd_str---屏幕输入指令
 * @return void
 * */
void interface_upload_client(char cmd_str[][64])
{
    // char trans_mode[20] = cmd_str[1];
    // char filename[20] = cmd_str[2];
    char trans_mode[64];
    char filename[64];
    strcpy(trans_mode, cmd_str[1]);
    strcpy(filename, cmd_str[2]);

    char save_file[BUFFSIZE] = {0}; /* 文件实际存放路径 */
    sprintf(save_file, "%s%s", cli_place_of_file, cmd_str[2]);

    /* TCP mode */
    if (0 == strcasecmp(trans_mode, "tcp"))
    {
        if (0 == strcasecmp(filename, ""))  /* filename为空 */
        {
            printf("\tWrong Command! Enter: 'upload TCP/UDP filename'\n");
        }
        else 
        {
            if (cli_tcp_sockfd < 0) /* 未连接server */
            {
                printf("\tNo available server! Please connect one first!\n");
            }
            else 
            {
                if (access(save_file, F_OK) != -1)   /* 判断文件是否存在 */
                {
                    struct stat s_buf;
                    stat(save_file, &s_buf);  /* 获取文件信息 */
                    if (S_ISREG(s_buf.st_mode)) /* 判断是文件还是目录 */
                    {
                        client_upload_tcp(save_file, filename);
                    }
                    else 
                    {
                        /* 目录，打印错误信息 */
                        printf("\tCan't upload a dir!Please input again\n");
                    }
                }
                else 
                {
                    /* 文件不存在 */
                    printf("\tPlease check if the file is exist!\n");
                }
            }
        }
    }
    /* udp mode */
    else if (0 == strcasecmp(trans_mode, "udp"))
    {
        if (0 == strcasecmp(filename, ""))  /* filename为空 */
        {
            printf("\tWrong Command! Enter: 'upload TCP/UDP filename'\n");
        }
        else
        {
            if (cli_udp_sockfd < 0)
            {
                printf("\tPlease check if udp server online!\n");
            }
            else
            {
                if (access(save_file, F_OK) != -1)
                {
                    struct stat s_buf;
                    stat(save_file, &s_buf);  /* 获取文件信息 */
                    if (S_ISREG(s_buf.st_mode)) /* 判断是文件还是目录 */
                    {
                        client_upload_udp(save_file, filename);
                    }
                    else 
                    {
                        /* 目录，打印错误信息 */
                        printf("\tCan't upload a dir!Please input again\n");
                    }
                }
                else
                {
                    /* 文件不存在 */
                    printf("\tPlease check if the file is exist!\n");
                }
                
            }
            
        }
        
    }
    else
    {
        printf("\tWrong Command! Enter: 'upload TCP/UDP filename'\n");
    }
    
}
/**
 * @brief 客户端上传模块实现接口
 * @param cmd_str---屏幕输入指令
 * @return void
 * */
void my_interface_upload_client(char cmd_str[][64])
{
    // char trans_mode[20] = cmd_str[1];
    // char filename[20] = cmd_str[2];
    char trans_mode[20];
    char filename[20];
    strcpy(trans_mode, cmd_str[1]);
    strcpy(filename, cmd_str[2]);

    char save_file[BUFFSIZE] = {0};
    sprintf(save_file, "%s%s", cli_place_of_file, cmd_str[2]);

    /* filename为空 */
    if (0 == strcasecmp(filename, ""))
    {
        printf("\tWrong Command! Enter: 'upload TCP/UDP filename'\n");
    }
    else
    {
        if ((access(save_file, F_OK)) != -1) 
        {
            /* 检索到待上传文件 */
            struct stat s_buf;
            stat(filename, &s_buf); /* 获取文件信息 */
            if (S_ISREG(s_buf.st_mode))
            {
                /* 文件, 选择上传操作 */
                /* tcp mode */
                if (0 == strcasecmp(trans_mode, "tcp"))
                {
                    if (cli_tcp_sockfd < 0) 
                    {
                        /* socket不正常，尚未建立TCP连接 */
                        printf("\tNo available server! Please connect one first!\n");
                    }
                    else
                    {
                        client_upload_tcp(save_file, filename);
                    }
                }
                /* udp mode */
                else if (0 == strcasecmp(trans_mode, "udp"))
                {
                    if (cli_udp_sockfd < 0)
                    {
                        /* 未连接udp服务器 */
                        printf("\tPlease check if udp server online!\n");
                    }
                    else
                    {
                        client_upload_udp(save_file, filename);
                    }
                    
                }
                else
                {
                    printf("\tWrong Command! Enter: 'upload TCP/UDP filename'\n");
                }
                
            }
            else
            {
                /* 目录，打印错误信息 */
                printf("\tCan't upload a dir!Please enter command again\n");
            }
            
        }
        else
        {
            printf("\tPlease check if the file is exist!\n");
        }
        
    }    
}


/**
 * @brief TCP模式下客户端上传文件至服务器
 *        socket使用全局sockfd，注意其是否已正确初始化
 *        上传发送消息格式为：
 *          "Msg_Type, FileName, FileSize, HashCode"
 * @param filename--文件名
 * @return 正常上传返回0, 异常返回ERROR
 * */
int client_upload_tcp(char* filename, char* singlefilename)
{
    /* 对传入文件名进行处理，统一修改成"./filename"格式 */
    char new_filename[BUFFSIZE] = {0};  /* 用于存储新的路径类型文件名 */
    if (NULL == strchr(filename, '/')) /* 没找到 */
    {
        sprintf(new_filename, "./%s", filename);
    }
    else 
    {
        strcpy(new_filename, filename);
    }

    /* 初始化文件信息结构 */
    struct FileInfo my_fileInfo;
    memset(&my_fileInfo, 0, sizeof(my_fileInfo));
    strcpy(my_fileInfo.fileName, new_filename);
    my_fileInfo.fileSize = get_file_size(my_fileInfo.fileName);
    /* 获取哈希码，待扩充 */
    strcpy(my_fileInfo.hashCode, MY_FAKE_HASHCODE); 

    char str[BUFFSIZE] = {0};
    sprintf(str, "%d,%s,%d,%s", MSG_TCP_UPLOAD, split_filename(my_fileInfo.fileName),
            my_fileInfo.fileSize, my_fileInfo.hashCode);
    printf("Sha1 Code: %s\n", my_fileInfo.hashCode);

    /* 发送要上传的文件信息 */
    send(cli_tcp_sockfd, str, strlen(str), 0);

    /* 接收服务端发来的断点文件信息 */
    recv(cli_tcp_sockfd, str, BUFFSIZE, 0); /* 信息格式：断点pos,大于0的值 无断点文件为0 */
    int point_pos = atoi(str);

    /* 打开文件 */
    FILE *fp;
    if (NULL == (fp = fopen(my_fileInfo.fileName, "rb")))
    {
        return ERROR;
    }
    /* 处理发送，从断点处开始还是新建发送 */
    char buf[BUFFSIZE + 1] = {0};
    fseek(fp, point_pos, SEEK_SET); /* 定位到服务端发来的断点 */
    int readSize = 0;   /* 读取长度 */
    int sendSize = 0;   /* 发送长度 */
    int completeSize = 0;   /* 记录已发送 */
    printf("Uploading...\n");
    while (1)
    {
        readSize = fread(buf, 1, BUFFSIZE, fp);
        if (0 == readSize)
        {
            break;
        }
        sendSize = send(cli_tcp_sockfd, (const char *)buf, readSize, 0);
        completeSize += sendSize;
        double donePer = 1.0 * completeSize / (my_fileInfo.fileSize - point_pos) * 100;
        // printf("%d of %d has sended!\n", completeSize, my_fileInfo.fileSize - point_pos);
        printf("Sending %s , %.02f%% completed...\n", singlefilename, donePer);
    }
    printf("Send completed! buf size = %dbytes\n", completeSize);
    fclose(fp);

    /* 等待服务端回传i消息 验证文件完整性 */
    if (0 == recv(cli_tcp_sockfd, (char *)buf, BUFFSIZE, 0))
    {
        /* 服务端传回出错 */
        return ERROR;
    }
    printf("Server: %s\n", buf);

    return 0;
}

/**
 * @brief UDP模式下客户端上传文件至服务器
 *        socket使用全局sockfd，注意其是否已正确初始化
 *        上传发送消息格式为：
 *          "Msg_Type, FileName, FileSize, HashCode"
 * @param filename--文件名
 * @return 正常上传返回0, 异常返回ERROR
 * */
int client_upload_udp(char* filename, char *singlefilename)
{
    /* 对传入文件名进行处理，统一修改成"./filename"格式 */
    char new_filename[BUFFSIZE] = {0};  /* 用于存储新的路径类型文件名 */
    if (NULL == strchr(filename, '/')) /* 没找到 */
    {
        sprintf(new_filename, "./%s", filename);
    }
    else 
    {
        strcpy(new_filename, filename);
    }

    /* 初始化文件信息结构 */
    struct FileInfo my_fileInfo;
    memset(&my_fileInfo, 0, sizeof(my_fileInfo));
    strcpy(my_fileInfo.fileName, new_filename);
    my_fileInfo.fileSize = get_file_size(my_fileInfo.fileName);
    /* 获取哈希码，待扩充 */
    strcpy(my_fileInfo.hashCode, "i have no hashcode now"); 

    char str[BUFFSIZE] = {0};
    sprintf(str, "%d,%s,%d,%s", MSG_UDP_UPLOAD, split_filename(my_fileInfo.fileName),
            my_fileInfo.fileSize, my_fileInfo.hashCode);
    printf("Sha1 Code: %s\n", my_fileInfo.hashCode);

    /* 发送要上传的文件信息 */
    sendto(cli_udp_sockfd, str, strlen(str), 0, (struct sockaddr *)&cli_udp_sockaddr, sizeof(cli_udp_sockaddr));

    /* 打开文件 */
    FILE *fp;
    if (NULL == (fp = fopen(my_fileInfo.fileName, "rb")))
    {
        return ERROR;
    }
    
    char buf[BUFFSIZE + 1] = {0};
    // fseek(fp, point_pos, SEEK_SET); /* 定位到服务端发来的断点 */
    int readSize = 0;   /* 读取长度 */
    int sendSize = 0;   /* 发送长度 */
    int completeSize = 0;   /* 记录已发送 */
    int addrSize = sizeof(cli_udp_sockaddr);
    printf("Uploading...\n");
    while (1)
    {
        readSize = fread(buf, 1, BUFFSIZE, fp);
        if (0 == readSize)
        {
            break;
        }
        sendSize = sendto(cli_udp_sockfd, (const char *)buf, readSize, 0, 
                            (struct sockaddr *)&cli_udp_sockaddr, sizeof(cli_udp_sockaddr));
        /* 等待服务器应答 */
        recvfrom(cli_udp_sockfd, buf, BUFFSIZE, 0, (struct sockaddr *)&cli_udp_sockaddr, &addrSize);

        completeSize += sendSize;
        double donePer = 1.0 * completeSize / my_fileInfo.fileSize * 100;
        // printf("%d of %d has sended!\n", completeSize, my_fileInfo.fileSize - point_pos);
        printf("Sending %s , %.02f%% completed...\n", singlefilename, donePer);
    }
    /* 发送消息，告知完成传输 */
    sendto(cli_udp_sockfd, "OK", 2, 0, (struct sockaddr *)&cli_udp_sockaddr, sizeof(cli_udp_sockaddr));

    printf("Send completed! buf size = %dbytes\n", completeSize);
    fclose(fp);

    /* 等待服务端回传消息 验证文件完整性 */
    memset(str, 0, sizeof(buf));
    if (0 == recvfrom(cli_udp_sockfd, buf, BUFFSIZE, 0, (struct sockaddr *)&cli_udp_sockaddr, &addrSize))
    {
        /* 服务端传回出错 */
        return ERROR;
    }
    printf("Server: %s\n", buf);

    return 0;
}

/**
 * @brief UDP模式下客户端上传文件至服务器
 *        socket使用全局sockfd，注意其是否已正确初始化
 *        上传发送消息格式为：
 *          "Msg_Type, FileName, FileSize, HashCode"
 * @param filename--文件名
 * @return 正常上传返回0, 异常返回ERROR
 * */
int backlog_client_upload_udp(char* filename)
{
    /* 对传入文件名进行处理，统一修改成"./filename"格式 */
    char new_filename[BUFFSIZE] = {0};  /* 用于存储新的路径类型文件名 */
    if (NULL == strchr(filename, '/')) /* 没找到 */
    {
        sprintf(new_filename, "./%s", filename);
    }
    else 
    {
        strcpy(new_filename, filename);
    }

    /* 初始化文件信息结构 */
    struct FileInfo my_fileInfo;
    memset(&my_fileInfo, 0, sizeof(my_fileInfo));
    strcpy(my_fileInfo.fileName, new_filename);
    my_fileInfo.fileSize = get_file_size(my_fileInfo.fileName);
    /* 获取哈希码，待扩充 */
    strcpy(my_fileInfo.hashCode, "i have no hashcode now"); 

    char str[BUFFSIZE] = {0};
    sprintf(str, "%d,%s,%d,%s", MSG_UDP_UPLOAD, split_filename(my_fileInfo.fileName),
            my_fileInfo.fileSize, my_fileInfo.hashCode);
    printf("Sha1 Code: %s\n", my_fileInfo.hashCode);

    /* 发送要上传的文件信息 */
    sendto(cli_udp_sockfd, str, strlen(str), 0, (struct sockaddr *)&cli_udp_sockaddr, sizeof(cli_udp_sockaddr));

    /* 打开文件 */
    FILE *fp;
    if (NULL == (fp = fopen(my_fileInfo.fileName, "rb")))
    {
        return ERROR;
    }
    
    char buf[BUFFSIZE + 1] = {0};
    // fseek(fp, point_pos, SEEK_SET); /* 定位到服务端发来的断点 */
    int readSize = 0;   /* 读取长度 */
    int sendSize = 0;   /* 发送长度 */
    int completeSize = 0;   /* 记录已发送 */
    int addrSize = sizeof(cli_udp_sockaddr);
    printf("Uploading...\n");
    while (1)
    {
        readSize = fread(buf, 1, BUFFSIZE, fp);
        if (0 == readSize)
        {
            break;
        }
        sendSize = sendto(cli_udp_sockfd, (const char *)buf, readSize, 0, 
                            (struct sockaddr *)&cli_udp_sockaddr, sizeof(cli_udp_sockaddr));
        /* 等待服务器应答 */
        recvfrom(cli_udp_sockfd, buf, BUFFSIZE, 0, (struct sockaddr *)&cli_udp_sockaddr, &addrSize);

        completeSize += sendSize;
        double donePer = 1.0 * completeSize / my_fileInfo.fileSize;
        // printf("%d of %d has sended!\n", completeSize, my_fileInfo.fileSize - point_pos);
        printf("Sending %s , %f%% completed...\n", my_fileInfo.fileName, donePer);
    }
    /* 发送消息，告知完成传输 */
    sendto(cli_udp_sockfd, "OK", 2, 0, (struct sockaddr *)&cli_udp_sockaddr, sizeof(cli_udp_sockaddr));

    printf("Send completed! buf size = %dbytes\n", completeSize);
    fclose(fp);

    /* 等待服务端回传消息 验证文件完整性 */
    memset(str, 0, sizeof(buf));
    if (0 == recvfrom(cli_udp_sockfd, buf, BUFFSIZE, 0, (struct sockaddr *)&cli_udp_sockaddr, &addrSize))
    {
        /* 服务端传回出错 */
        return ERROR;
    }
    printf("Server: %s\n", buf);

    return 0;
}