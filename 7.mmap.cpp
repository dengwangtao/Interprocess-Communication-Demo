#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

const int MAP_SIZE = 2048; // 内存映射的大小

struct DATA {
    bool readable; // 是否写入了数据(true: 可读, false, 可写)
    int id;
    char message[MAP_SIZE - sizeof(bool) - sizeof(int)];
};

int main() {

    // 打开一个文件
    int fd = open("7.map.txt", O_RDWR | O_CREAT, 0664);
    if(fd == -1) {
        perror("open(\"7.map.txt\", O_RDWR | O_CREAT, 0664)");
        return -1;
    }

    // 调整文件大小
    if(ftruncate(fd, MAP_SIZE) == -1) {
        perror("ftruncate(fd, MAP_SIZE)");
        close(fd);
        return -1;
    }

    // 内存映射 (也可以不映射文件)
    void * ptr = mmap(nullptr, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ptr == MAP_FAILED) {
        perror("mmap(nullptr, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)");
        close(fd);
        return -1;
    }
    memset(ptr, 0, MAP_SIZE); // 置为0

    int pid = fork();

    if(pid > 0) {
        // parent read
        DATA *data = reinterpret_cast<DATA*>(ptr);
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

        waitpid(-1, nullptr, 0);

    } else if (pid == 0) {
        // child write
        DATA* data = reinterpret_cast<DATA*>(ptr);
        for(int i = 9; i >= 0;) {
            if(data->readable == false) { // 不可读 就是 可写
                data->id = i;
                memset(data->message, 0, sizeof(data->message));
                sprintf(data->message, "message %d", i);
                data->readable = true; // 设置可读
                -- i;
            }
        }

        return 0;
    }

    // 解除内存映射
    if (munmap(ptr, MAP_SIZE) == -1) {
        perror("munmap");
    }
    close(fd);
    return 0;
}