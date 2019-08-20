/**
 * @file    tools.c
 * @brief   ���ܺ����Ķ���ʵ��
 * @author  lizidong
 * @date    2019/8/14
 * */
#include "tools.h"

/*  �޻��Զ�ȡ�ַ� */
int getch(void)
{
    struct termios tm, tm_old;
    int fd = 0, ch;
    /* �������ڵ��ն����� */
    if (tcgetattr(fd, &tm) < 0) 
    {
        return -1;
    }
    tm_old = tm;
    /* �����ն�����Ϊԭʼģʽ����ģʽ�����е������������ֽ�Ϊ��λ������ */
    cfmakeraw(&tm);
    /* �����ϸ���֮������� */
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
    {
        return -1;
    }

    ch = getchar();

    /* ��������Ϊ��������� */
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
    {
        return -1;
    }

    return ch;
}

/**
 * @brief �����ļ��洢·��
 * @param �ļ�Ŀ¼
 * @return void
 * */
void mk_folder_of_file(char *file_dir)
{
    if (NULL == opendir(file_dir))
    {
        mkdir(file_dir, 0777);  /* �ļ�������ӵ�ж���д��ִ�в�����Ȩ�� */
    }
}

/**
 * @brief �����и������������������кʹ�ӡ���
 * @param void
 * @return void
 * */
void hightlight_Command()
{
    printf("\033[1;32mlzd@localhost:~\033[0m");
    printf("\033[0;34m$ \033[0m");
}

/**
 * @brief ��ȡ�ļ���С
 * @param �ļ�·����
 * @return �ļ���С
 * */
int get_file_size(char* filename)
{
    struct stat statbuff;
    stat(filename, &statbuff);
    return statbuff.st_size;
}

/**
 * @brief ��·���з���������ļ���
 * @param �ļ�·����
 * @return �ļ���
 * */
char* split_filename(char *filename)
{
    char ch = '/';
    char *file_single_name = strrchr(filename, ch) + 1;
    return file_single_name;
}