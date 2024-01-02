#include <semaphore.h> 
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

const int PRODUCER_NUM = 5;
const int NUM = 3;

int main() {

    const char* sem_name = "8.semaphore"; // 文件存放在 /dev/shm/sem.8.semaphore
    sem_t *sem = sem_open(sem_name, O_CREAT | O_EXCL | O_RDWR, 0666, 0); // 信号量初始值为1
    if(SEM_FAILED == sem) {
        perror("sem_open");
        return -1;
    }

    int pid = 0;
    for(int i = 1; i <= PRODUCER_NUM; ++ i) { // 创建5个子进程
        pid = fork();
        if(pid == 0) break;
    }

    if(pid > 0) {
        // parent (customer) 一个
        int count = 0;
        while(1) {
            sem_wait(sem);
            printf("消费者消费一个资源 %d\n", ++ count);
            if(count >= PRODUCER_NUM * NUM) {
                break;
            }
        }
    }
    
    if(pid == 0) {
        //child (producer) 多个
        for(int i = 0; i < NUM; ++ i) {
            sleep(rand() % 3 + 1);
            sem_post(sem); // 生产一个资源
        }
    }


    if(pid > 0) {
        // 关闭并删除命名信号量
        sem_close(sem);
        sem_unlink(sem_name);
    }

    return 0;
}