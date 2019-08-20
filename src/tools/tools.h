#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#define MY_FAKE_HASHCODE "i have no hashcode now"
/* 定义客户端发送消息类型 */
enum Msg_Type
{
    MSG_TCP_LS,         /* 请求服务端文件列表 */
    MSG_TCP_DOWNLOAD,   /* TCP模式下载 */
    MSG_TCP_UPLOAD,     /* TCP模式上传 */
    MSG_UDP_DOWNLOAD,   /* UDP模式下载 */
    MSG_UDP_UPLOAD,     /* UDP模式上传 */
};

/* 无回显读取字符 */
int getch(void);    

/* 创建文件存放路径 */
void mk_folder_of_file(char *file_dir);

/* 命令行高亮，区分命令输入行和打印输出 */
void hightlight_Command();

/* 获取文件大小 */
int get_file_size(char* filename);

/* 从路径中分离出单个文件名 */
char* split_filename(char *filename);