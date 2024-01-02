#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <limits.h>

long long idx = 0;

void handler(int sig) {
    static int count = 0;
    count += 1;

    printf("SIGALARM信号 触发第%d次 idx=%lld\n", count, idx);
    if(count == 3) {
        printf("程序退出\n");
        exit(0);
    }
}

int main() {

    // 绑定信号处理函数
    signal(SIGALRM, handler);

    itimerval interval{{2, 0}, {1, 0}}; // 每次间隔2秒, 延迟1秒执行

    if(setitimer(ITIMER_REAL, &interval, NULL) == -1) { // 真实时间 
        perror("setitimer(ITIMER_REAL, &interval, NULL)");
        exit(-1);
    }

    while(idx < LLONG_MAX) {
        idx += 1;
    }

    return 0;
}