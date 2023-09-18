#include "stdio.h"
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
const int64_t BUFFER_SIZE = 104857600;
const int32_t HEADER_SIZE = 200;
const int32_t FILE_SIZE = 200;
const int32_t FD_SIZE = 2;
#define DEBUG false

struct
{
    int maxi;
    struct pollfd pfds[FD_SIZE];

} pool;
void check_ret(int ret)
{
    if (ret == -1)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(-1);
    }
}

void getType(char *pathname, char *header, char *type)
{
    char *p = strtok(pathname, ".");
    p = strtok(NULL, ".");
#if DEBUG
    printf("%s\n", p);
#endif
    if (strcmp(p, "txt") == 0)
    {
        strcpy(type, "text/plain");
    }
    else if ((strcmp(p, "jpg") == 0) || (strcmp(p, "jpeg") == 0))
    {
        strcpy(type, "image/jpeg");
    }
    else if (strcmp(p, "png") == 0)
    {
        strcpy(type, "image/png");
    }
    else if (strcmp(p, "html") == 0)
    {
        strcpy(type, "text/html");
    }
    else
    {
        strcpy(type, "application/octet-stream");
    }
}
bool is_file(char *pathname)
{
    if (!pathname)
    {
        printf("file is NULL\n");
        return false;
    }
    char buf[FILE_SIZE] = "";
    strncpy(buf, pathname, sizeof(buf));
    char *ptr = strtok(buf, ".");
    ptr = strtok(NULL, ".");
    if (ptr == NULL)
    {
        return false;
    }
    return true;
}
void build_response(char *response, char *pathname, size_t *response_len)
{
    if (!pathname | !response)
    {

        snprintf(response, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n404 Not Found");
        *response_len = strlen(response);
        return;
    }

    int fd = open(pathname, O_RDONLY);
    if (fd == -1)
    {
        printf("Can not open %s\n", pathname);
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n404 Not Found");
        *response_len = strlen(response);
        return;
    }
    char type[20] = "";
    char *header = (char *)calloc(HEADER_SIZE, sizeof(char));

    struct stat file_stat;
    int r = fstat(fd, &file_stat);
    assert(r == 0);
    off_t file_len = file_stat.st_size;

    getType(pathname, header, type);
    snprintf(header, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n", file_len, type);
    strncpy(response, header, strlen(header));
    memcpy(response, header, strlen(header));
    *response_len += strlen(header);

    ssize_t bytes_read;
    while ((bytes_read = read(fd,
                              response + *response_len,
                              BUFFER_SIZE - *response_len)) > 0)
    {
        *response_len += bytes_read;
    }
    free(header);
    close(fd);
}

void handle_exec(char *pathname, int fd)
{
    char *execute = strtok(pathname, "?");
    char *query_string = strtok(NULL, "?");
    char *new_argv[2];
    char *new_envp[2];
    char temp[100] = "";
    sprintf(temp, "QUERY_STRING=%s", query_string);
    new_argv[0] = execute;
    new_argv[1] = NULL;
    new_envp[0] = temp;
    new_envp[1] = NULL;
    pid_t child = fork();
    if (child == 0)
    {
        int r = dup2(fd, 1);
        r = execve(execute, new_argv, new_envp);
        check_ret(r);
    }
    else
    {
        int status;
        wait(&status);
    }
}

void client_func(int conn_fd)
{
    char buf[1024] = "";
    recv(conn_fd, buf, sizeof(buf), 0);
#if DEBUG
    printf("Buf : %s", buf);
#endif
    char *path = strtok(buf, " ");
    path = strtok(NULL, " ");
    char *response = (char *)calloc(BUFFER_SIZE, sizeof(char));
    size_t response_len = 0;
    if (path == nullptr)
    {
        printf("Ask for Null \n");
        close(conn_fd);
    }
    if (is_file(path))
    {
#if DEBUG
        printf("this %s \n", path);
#endif
        build_response(response, path, &response_len);
        send(conn_fd, response, response_len, 0);
    }
    else
    {
#if DEBUG
        printf("Enter %s \n", path);
#endif
        handle_exec(path, conn_fd);
    }
    printf("%s\n", response);
    free(response);
}

int main()
{
    int ret, listen_fd, on;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    check_ret(listen_fd);
    on = 1;
    ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in ser_addr,
        client_addr;
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(25001);
    ser_addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t client_len = sizeof(client_addr);
    ret = bind(listen_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));

    check_ret(ret);
    ret = listen(listen_fd, 20);

    check_ret(ret);
    pool.maxi = 0;
    pool.pfds[pool.maxi].fd = listen_fd;
    pool.pfds[pool.maxi++].events = POLLIN;
    printf("Server on  %s:%d\n", inet_ntoa(ser_addr.sin_addr), ntohs(ser_addr.sin_port));
    for (int i = 1; i < FD_SIZE; ++i)
    {
        pool.pfds[i].fd = -1;
    }
    while (1)
    {
        ret = poll(pool.pfds, std::min(pool.maxi, FD_SIZE), -1);

        if (pool.pfds[0].revents & POLLIN)
        {
#if DEBUG
            printf("Begin to accept\n");
#endif
            int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
            int k;
            for (k = 1; k < FD_SIZE; ++k)
            {
                if (pool.pfds[k].fd < 0)
                {
                    pool.pfds[k].fd = conn_fd;
                    pool.pfds[k].events = POLLIN;
                    pool.maxi++;
                    break;
                }
            }
            if (k >= FD_SIZE)
            {
                close(conn_fd);
                fprintf(stderr, "Too much clients. Lost connection\n");
            }
            else
            {
                printf("Connect %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            }
        }
        for (int i = 1; i < FD_SIZE; ++i)
        {
            if (pool.pfds[i].fd < 0)
            {
                continue;
            }
            if (pool.pfds[i].revents & POLLIN)
            {
                client_func(pool.pfds[i].fd);
                close(pool.pfds[i].fd);
                pool.pfds[i].fd = -1;
            }
        }
    }
    close(listen_fd);
}