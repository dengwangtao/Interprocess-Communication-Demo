#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

int main() {

    const char* FIFO_NAME = "./2.fifo";

    if(access(FIFO_NAME, F_OK) != 0) { // 2.fifo文件不存在, 则创建
        printf("%s不存在, 正在创建\n", FIFO_NAME);
        // 创建fifo文件
        if(mkfifo(FIFO_NAME, 0664) == -1) {
            perror("mkfifo");
            return -1;
        }
    }    

    // 创建进程
    pid_t pid = fork();
    if(pid > 0) {
        // parent
        int fd = open(FIFO_NAME, O_RDONLY);

        char buf[128] = {0};
        
        size_t len = 0;
        while((len = read(fd, buf, sizeof(buf))) > 0) {
            printf("父进程收到: %s\n", buf);
            memset(buf, 0, sizeof(buf));
        }
        
        wait(NULL);

        close(fd);
    } else if(pid == 0) {
        // son

        int fd = open(FIFO_NAME, O_WRONLY);
        char buf[128] = {0};

        for(int i = 0; i < 10; ++ i) {
            sprintf(buf, "message %d", i);
            write(fd, buf, strlen(buf));
            memset(buf,  0, sizeof(buf));
            sleep(1);
        }

        close(fd);

    }

    return 0;
}