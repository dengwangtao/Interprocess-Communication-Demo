#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>

#define SOCKET_FILE "./socket_local.sock"

int main() {

    // 1. 创建套接字
    int lfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(lfd == -1) {
        perror("socket");
        return -1;
    }

    // 服务端套接字
    sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, SOCKET_FILE);

    // 2. connect()
    if(connect(lfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        perror("connect");
        close(lfd);
        return -1;
    }

    // 3.发送数据
    char buf[128] = {0};
    int pid = getpid();
    printf("客户端进程: %d\n", pid);
    for(int i = 0; i < 10; ++ i) {
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "message %d from %d", i, pid);
        if(send(lfd, buf, strlen(buf), 0) == -1) {
            perror("send");
            close(lfd);
            return -1;
        }
        sleep(1);
    }

    close(lfd);
}