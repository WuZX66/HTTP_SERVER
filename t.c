#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(int argc, char *argv[], char *envp[])
{
    int fd = open("a.txt", O_RDWR);
    if (fd == -1)
    {
        fprintf(stderr, "Can not open file \n");
        exit(-1);
    }
    char *new_argv[2];
    char *new_envp[2];
    char p[] = "/home/wzx/Project/network/MyNet/test";
    char str[] = "1&2";
    char n[100] = "";
    sprintf(n, "QUERY_STRING=%s", str);
    new_argv[0] = p;
    new_argv[1] = NULL;
    new_envp[0] = n;
    new_envp[1] = NULL;

    pid_t child = fork();
    if (child == 0)
    {
        int r = dup2(fd, 1);
        close(fd);
        r = execve(p, new_argv, new_envp);
        assert(r != -1);
        //    fflush(stdout);
        //    write(1, buf, sizeof (buf));
    }
    else
    {
        int status;
        wait(&status);
    }
    return 0;
}