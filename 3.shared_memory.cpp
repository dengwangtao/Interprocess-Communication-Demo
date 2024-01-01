#include <errno.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

const key_t KEY = 123;
const int MEM_SIZE = 128;

struct DATA {
    bool readable; // 是否写入了数据(true: 可读, false, 可写)
    int id;
    char message[MEM_SIZE];
};

int main() {

    // 获取共享内存段, key相同, 获取的内存就相同
    // IPC_EXCL: 如果重复创建key=123的共享内存, 就报错
    int shmid = shmget(KEY, sizeof(DATA), IPC_CREAT | IPC_EXCL | 0664);
    if (shmid == -1) {
        perror("shmget(KEY, sizeof(DATA), IPC_CREAT | IPC_EXCL | 0664)");
        return -1;
    }

    int pid = fork();

    if(pid > 0) {
        // parent
        void* ptr = shmat(shmid, NULL, 0);
        DATA* data = (DATA*)ptr;
        while(1) {
            if(data->readable) { // 可以读
                printf("id=%d, message=%s\n", data->id, data->message);
                sleep(1); // 模拟处理数据的时间
                if(data->id <= 0) {
                    // 数据结束
                    break;
                }
                data->readable = false; // 读取完毕, 可以写了
            }
        }

        wait(NULL);

    } else if(pid == 0) {
        // child
        void* ptr = shmat(shmid, NULL, 0);
        DATA* data = (DATA*)ptr;

        for(int i = 9; i >= 0;) {
            if(data->readable == false) { // 不可读 就是 可写
                data->id = i;
                memset(data->message, 0, sizeof(data->message));
                sprintf(data->message, "message %d", i);
                data->readable = true; // 设置可读
                -- i;
            }
        }
    }

    if(pid > 0) { // 删除共享内存
        shmctl(shmid, IPC_RMID, NULL);
        printf("已删除共享内存(shmid=%d)\n", shmid);
    }
    
    return 0;
}