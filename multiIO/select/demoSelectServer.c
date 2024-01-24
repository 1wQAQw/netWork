#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>



#define SERVER_PORT 8080
#define MAX_LISTEN 128
#define BUFFER_SIZE 128
/* 用单进程/线程 实现并发 */
int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    /* 将本地的IP端口绑定 */
    struct sockaddr_in localAddress;
    bzero(&localAddress, sizof(localAddress));
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(SERVER_PORT);
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t localAddressLen = sizeof(localAddress);
    int ret = bind(sockfd, (struct sockaddr_in *)&localAddress, localAddressLen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* 监听 */
    ret = listen(sockfd, MAX_LISTEN);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    /* 把监听大的文件描述符号添加到读集合中 */
    fd_set readSet;
    /* 清空集合 */
    FD_ZERO(&readSet);
    /* 把监听的文件描述符添加到读集合中， 让内核帮忙检测 */
    FD_SET(sockfd, &readSet);

    int maxfd = sockfd;
    while (1)
    {
        select(maxfd + 1, &readSet, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select error");
            break;
        }

        /* 如果sockfd在readset集合里面 */
        if (FD_ISSET(sockfd, &readSet))
        {
            int acceptfd = accept(sockfd, NULL, NULL);
            if (accept == -1)
            {
                perror("accept error");
                break;
            }
            /* 将通讯的句柄 放到读集合 */
            FD_SET(acceptfd, &readSet);


            maxfd = maxfd < acceptfd ? acceptfd : maxfd;
        }

        for (int idx = 0; idx <= maxfd; idx++)
        {
            if (idx != sockfd && FD_ISSET(idx, &readSet))
            {
                char buf[BUFFER_SIZE];
                bzero(buf, sizeof(buf));
                /* 程序到这里，一定有通讯（老客户）*/
                int readByte = read(idx, buf, sizeof(buf) - 1);
                if (readByte < 0)
                {
                    perror("read error");
                    /**/
                    FD_CLR(idx, &readSet);
                    /* 关闭文件句柄 */
                    close(idx);
                    /* 这边要做成continue..., 让下一个已ready的fd句柄进行通信 */
                    continue;
                }
                else if (readByte == 0)
                {
                    printf("客户端断开连接...\n");
                    /**/
                    FD_CLR(idx, &readSet);
                    /* 关闭通讯句柄 */
                    close(idx);
                    continue;
                }
                else
                {
                    printf("recv:%s\n", buf);

                    for (int jdx = 0; jdx < readByte; jdx++)
                    {
                        buf[jdx] = toupper(buf[jdx]);
                    }

                    write(idx, buf, sizeof(buf) - 1);
                    usleep(100);
                }
            }
        }




    }

    /* 关闭文件描述符 */

    return 0;
}