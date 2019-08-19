/**
 * @file    tools.c
 * @brief   功能函数的定义实现
 * @author  lizidong
 * @date    2019/8/14
 * */
#include "tools.h"

/*  无回显读取字符 */
int getch(void)
{
    struct termios tm, tm_old;
    int fd = 0, ch;
    /* 保存现在的终端设置 */
    if (tcgetattr(fd, &tm) < 0) 
    {
        return -1;
    }
    tm_old = tm;
    /* 更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理 */
    cfmakeraw(&tm);
    /* 设置上更改之后的设置 */
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
    {
        return -1;
    }

    ch = getchar();

    /* 更改设置为最初的样子 */
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
    {
        return -1;
    }

    return ch;
}

/**
 * @brief 创建文件存储路径
 * @param 文件目录
 * @return void
 * */
void mk_folder_of_file(char *file_dir)
{
    if (NULL == opendir(file_dir))
    {
        mkdir(file_dir, 0777);  /* 文件所有者拥有读，写和执行操作的权限 */
    }
}

/**
 * @brief 命令行高亮，区分命令输入行和打印输出
 * @param void
 * @return void
 * */
void hightlight_Command()
{
    printf("\033[1;32mlzd@localhost:~\033[0m");
    printf("\033[0;34m$ \033[0m");
}

/**
 * @brief 获取文件大小
 * @param 文件路径名
 * @return 文件大小
 * */
int get_file_size(char* filename)
{
    struct stat statbuff;
    stat(filename, &statbuff);
    return statbuff.st_size;
}

/**
 * @brief 从路径中分离出单个文件名
 * @param 文件路径名
 * @return 文件名
 * */
char* split_filename(char *filename)
{
    char ch = '/';
    char *file_single_name = strrchr(filename, ch) + 1;
    return file_single_name;
}