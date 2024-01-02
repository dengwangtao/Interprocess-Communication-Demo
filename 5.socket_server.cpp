#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <csignal>
#include <chrono>

#define SOCKET_FILE "./socket_local.sock"

bool exit_flag = false;

void quitHandler(int sig) {
    if(sig == SIGINT) {
        exit_flag = true;
        printf("收到 ctrl + C 信号\n");
    }
}

void handleClient(int client_sock, sockaddr_un client_addr) {
    printf("%d 已连接, 当前线程: %ld\n", client_sock, std::this_thread::get_id());
    char data[128];
    ssize_t len = 0;
    while (true) {
        len = recv(client_sock, data, sizeof(data), 0);
        if(len == -1) {
            printf("%d 号连接recv出错\n", client_sock);
            perror("recv");
            return;
        }
        if(len == 0) {
            printf("%d 号连接已断开\n", client_sock);
            return;
        }
        printf("%d 号连接 收到数据: %s\n", client_sock, data);
    }
    
}

int main() {

    std::signal(SIGINT, quitHandler);

    // 删除可能已存在的套接字文件
    unlink(SOCKET_FILE);

    // 1. 创建套接字 (非阻塞)
    int lfd = socket(AF_LOCAL, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(lfd == -1) {
        perror("socket");
        return -1;
    }

    // 本地通信套接字
    sockaddr_un addr;
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, SOCKET_FILE);

    // 2. bind()
    int ret = bind(lfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if(ret == -1) {
        perror("bind(lfd, reinterpret_cast<sockaddr*>(addr), sizeof(addr))");
        close(lfd);
        unlink(SOCKET_FILE);
        return -1;
    }

    // 3. listen
    if(listen(lfd, 5)) {
        perror("listen(lfd, 5)");
        close(lfd);
        unlink(SOCKET_FILE);
        return -1;
    }

    // 通信
    while(!exit_flag) {
        sockaddr_un client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // 4. accept
        int cfd = accept(lfd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
        if(cfd == -1) {
            if(errno == EWOULDBLOCK) {
                // 非阻塞套接字无连接
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                // printf("check 无连接\n");
                continue;
            }
            perror("accept");
            break;
        }

        std::thread handler_th(handleClient, cfd, client_addr);
        handler_th.detach();
    }

    close(lfd);
    unlink(SOCKET_FILE); // 删除套接字文件

    return 0;
}