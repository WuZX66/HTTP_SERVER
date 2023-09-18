#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
using namespace std;


int main(int argc, char *argv[])
{

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    cout << "fd: " << fd << endl;
    if (fd < 0)
    {
        cout << "fd < 0 ..." << endl;
        exit(1);
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(25001);
    addr.sin_addr.s_addr = INADDR_ANY;
    cout << "Trying to connect to " << inet_ntoa(addr.sin_addr) << ": " << ntohs(addr.sin_port) << endl;
    int r = connect(fd, (sockaddr *)&addr, sizeof(addr));
    if (r < 0)
    {
        cout << "connect failed ..." << endl;
        printf("Error : %s\n", strerror(errno));
        exit(-1);
    }
   
    int num_fds = 2;
    struct pollfd *pfds;
    pfds = (pollfd *)calloc(num_fds, sizeof(pollfd));
    if (pfds == nullptr)
    {
        exit(-1);
    }
    pfds[0].fd = fd;
    pfds[0].events = POLLIN;
    pfds[1].fd = 0;
    pfds[1].events = POLLIN;

    int ready = poll(pfds, num_fds, -1);
    if (ready < 0)
    {
        exit(-1);
    }
    char host[200] = "";
    char RX[1024] = "";
    char TX[1024] = "";
    while (1)
    {
        if (pfds[1].revents != 0)
        {
            printf("fd = %d; events: %s", pfds[1].fd, (pfds[1].revents & POLLIN ? "POLLIN\n" : ""));
            if (pfds[1].revents & POLLIN)
            {
                cin >> host;
                char *hostname = strtok(host, "/");
                char *path = hostname + strlen(hostname) + 1;
                sprintf(RX, "GET /%s HTTP/1.1\r\nHost: %s\r\n", path, hostname);
                send(fd, RX, sizeof (RX), 0);
                recv(fd, TX, sizeof (TX), 0);
            }
            break;
        }
        else if (pfds[0].revents != 0)
        {
            cout << "TCP Server over , bye~\n";
            exit(1);
        }
    }

    return 0;
}