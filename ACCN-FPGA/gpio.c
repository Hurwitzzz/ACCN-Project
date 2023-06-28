#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>

int main() {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    // if(fd < 0) {
    //     perror("open");
    //     return -1;
    // }

    size_t page_addr = 0x41200000 & ~(sysconf(_SC_PAGE_SIZE) - 1);
    size_t buffer_length = 8;

    char* buffer = mmap(NULL, buffer_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_addr);

    // if(buffer == MAP_FAILED) {
    //     perror("mmap");
    //     close(fd);
    //     return -1;
    // }

    buffer[4] = 0;
    buffer[0] = 0;
    printf("successful\n");

    if(munmap(buffer, buffer_length) == -1) {
        perror("munmap");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}
