/*
当
    - 子进程终止
    - 子进程收到SIGSTOP信号停止
    - 子进程收到SIGCONT信号继续执行
时, 都会产生SIGCHLD信号发送至父进程, 父进程默认忽略
*/

#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void handle(int sig) {
    static int count = 0;
    printf("捕获到子进程结束\n", sig);
    while(1) { // SIGCHLD信号只会被触发一次, 所以要循环wait
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if(pid > 0) {
            printf("已关闭%d个子进程\n", ++ count);
        }
        if(pid <= 0) break;
    }
}

int main() {

    sigset_t set;                       // 声明一个信号集
    sigemptyset(&set);                  // 初始化信号集, 全部置0
    sigaddset(&set, SIGCHLD);           // 添加 SIGCHLD 到信号集
    sigprocmask(SIG_BLOCK, &set, NULL); // 设置阻塞信号集(此处只阻塞SIGCHLD信号)

    pid_t pid = 0;
    int i = 0;
    for (i = 0; i < 20; ++i) { // 创建20个子进程
        pid = fork();
        if (pid == 0) break;  //如果是子进程, 就不循环
    }

    struct sigaction sigact;
    sigact.sa_flags = 0;
    sigact.sa_handler = handle;
    sigemptyset(&sigact.sa_mask); // 在执行sa_handler函数时, 是否阻塞其他某些信号(此处不阻塞, 全置为零)
    
    sigaction(SIGCHLD, &sigact, NULL); // 设置信号处理函数

    if(pid == 0) {
        sleep(rand() % 3 + 1);
        printf("child process, pid = %d\n", getpid());
        return 0;
        // 子进程直接结束
    }

    // 当所有子进程创建完毕后, 解除阻塞SIGCHLD
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    
    while(1) {
        sleep(1);
    }


    return 0;
}