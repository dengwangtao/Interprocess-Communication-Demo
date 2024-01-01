#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

const key_t KEY = 123;
const int MAX_SIZE = 128;
const int P_NUM = 5; // 进程数量

struct MSG {
    long mtype;	// 一定大于0
    char mtext[MAX_SIZE]; // 消息
};


int main() {
    
    // 创建消息队列
    int mqid = msgget(KEY, IPC_CREAT | IPC_EXCL | 0664);
    if(mqid == -1) {
        perror(" msgget(KEY, IPC_CREAT | IPC_EXCL | 0664);");
        return -1;
    }

    pid_t pid = 0;
    long IDX = 1L;
    for(IDX = 1L; IDX <= P_NUM; ++ IDX) { // 创建P_NUM个子进程
        pid = fork();
        if(pid == 0) { // 子进程
            break;
        }
    }

    if(pid > 0) {
        
        // parent
        // 负责 发送消息

        for(int i = 0; i < 30; ++ i) {
            /**
             * __msgflg: 控制消息发送的标志位，可以是以下之一或它们的组合：
             *      IPC_NOWAIT: 如果消息队列已满，则立即返回，而不等待。
             *      MSG_NOERROR: 如果消息大小超过__msgsz，则截断消息，而不返回错误。
            */
            MSG msg;
            msg.mtype = i % P_NUM + 1;
            memset(msg.mtext, 0, sizeof(msg.mtext));
            sprintf(msg.mtext, "type=%d, message %d", msg.mtype, i);
            int ret = msgsnd(mqid, &msg, sizeof(msg), 0);
        }
        for(int i = 0; i < P_NUM; ++ i) { // 发送结束消息
            MSG msg;
            msg.mtype = i % P_NUM + 1;
            memset(msg.mtext, 0, sizeof(msg.mtext));
            msg.mtext[0] = '$';
            int ret = msgsnd(mqid, &msg, sizeof(msg), 0);
        }
    }
    

    if(pid == 0) {

        // child
        printf("IDX = %d, pid = %d\n", IDX, getpid());
        // child进程只接收 IDX对应的消息
        
        /**
         * __msgtyp: 指定要接收的消息类型。如果设置为0，则表示接收队列中的第一条消息。如果设置为正整数，则表示接收队列中第一个类型为__msgtyp的消息。如果设置为负值，则表示接收队列中类型值小于或等于__msgtyp的第一条消息
         * 
         * __msgflg: 控制消息接收的标志位，可以是以下之一或它们的组合：
         *    IPC_NOWAIT: 如果消息队列中没有符合条件的消息，则立即返回，而不等待。
         *    MSG_EXCEPT: 接收消息队列中第一个不等于__msgtyp的消息。
         *    MSG_NOERROR: 如果消息大小超过__msgsz，则截断消息，而不返回错误。
        */
        while(1) {
            MSG msg;
            ssize_t len = msgrcv(mqid, &msg, sizeof(msg), IDX, 0); // 只接受type为IDX的消息
            if(len == 0 || msg.mtext[0] == '$') { // 消息发送结束
                printf("进程%d (IDX=%d) 结束\n", getpid(), IDX);
                break;
            }
            printf("进程%d (IDX=%d), 接受到消息 (type=%ld):%s\n", getpid(), IDX, msg.mtype, msg.mtext);
            sleep(1);
        }
    }




    if(pid > 0) {
        for(int i = 0; i < P_NUM; ++ i) { // 等待所有子进程结束
            pid_t p = waitpid(-1, NULL, 0);
            printf("%d进程已关闭\n", p);
        }
        // 最后 关闭消息队列
        msgctl(mqid, IPC_RMID, NULL);
    }
    return 0;
}