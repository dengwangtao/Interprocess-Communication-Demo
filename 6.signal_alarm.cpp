/*
SYNOPSIS
       #include <unistd.h>
       unsigned int alarm(unsigned int seconds);
DESCRIPTION
       alarm() arranges for a SIGALRM signal to be delivered to the calling process in seconds seconds.
       If seconds is zero, any pending alarm is canceled.
       In any event any previously set alarm() is canceled.
RETURN VALUE
       alarm() returns the number of seconds remaining until any previously scheduled alarm was due to be delivered, or zero if there was no previously scheduled alarm.
*/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void alarm_handler(int sig) { // 收到SIGALARM信号后, 退出程序
    printf("收到ALARM信号, exit;\n");
    exit(0);
}

int main() {
    // 绑定SIGALRM信号处理函数
    signal(SIGALRM, alarm_handler);

    alarm(2); // 2秒后 向当前进程发送SIGALRM信号

    while(1) {
        printf("waiting...\n");
        sleep(1);
    }
    
    return 0;
}
