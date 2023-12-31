// #include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <string>

// using std::cout;
// using std::endl;



int main() {

    // 创建管道
    int pip[2] = {0};
    if(pipe(pip) == -1) { // 0: read | 1: write
        perror("pipe");
        return -1;
    }

    // 创建子进程
    pid_t pid = fork();
    if(pid == -1) {
        perror("fork");
        return -1;
    }

    if(pid > 0) { // 父进程 读管道
        printf("parent %d\n", getpid());
        close(pip[1]); // 关闭写
        
        char buf[128] = {0};
        ssize_t len = 0;
        while((len = read(pip[0], buf, sizeof(buf) - 1)) > 0) {
            printf("父进程收到数据 len=%d: %s\n", len, buf);
            memset(buf, 0, sizeof(buf));
        }
        if(len == -1) {
            perror("read");
            return -1;
        }

        wait(NULL);

    } else if(pid == 0) { // 子进程 写管道
        printf("son %d\n", getpid());
        close(pip[0]); // 关闭读
        char msg[128] = {0};

        for(int i = 1; i < 100; ++ i) {
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "message %d", i);
            int len = write(pip[1], msg, strlen(msg));

            if(len == -1) {
                perror("write");
                return -1;
            }

            sleep(1);
        }

    }

    return 0;
}
