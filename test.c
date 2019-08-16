#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"
void menu();
int main()
{
    // cmd_pre();
    // char *SysCommand[] = {"help", "detect", "connect", "ls_client", "ls_server",
                        // "download", "upload", "quit"};
    // printf("%s", SysCommand[0]);
    // if (getchar() == 'q')
    // {
        // printf("quit\n");
    // }
    // print();
    // menu();
    // char *ch;
    // scanf("%s", ch);
    // if (0 == strcmp(ch, "clear")) {
        // system("clear");
    // }
    printf("%s\n", MY_FAKE_HASHCODE);
    printf("%s%%\n", "test");
    printf("%s\n", split_filename("./././hike"));
    printf("%s\n", "./hike");
    printf("%s\n", split_filename(".///hik"));
    return 0;
}

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