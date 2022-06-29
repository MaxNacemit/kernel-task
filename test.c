#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        fd = open("/dev/my_device_driver", "w");
        char buf[] = "abcdef";
        if (write(fd, buf, 6) != 6) {
            exit(1);
        }
        char buf2[4096];
        for (int i = 0; i < 4095; ++i) {
            buf2[i] = '0';
        }
        buf2[4095] = 0;
        if (write(fd, buf2, 4095) < 0) {
            exit(1);
        }
        if (write(fd, buf, 6) < 0) {
            exit(1);
        }
        exit(0);
    } else {
        fd = open("/dev/my_device_driver", "r");
        char buf* = "mnklgh";
        read(fd, buf, 6);
        printf("%s\n", buf);
        char buf2[4096];
        char readbuf[4096];
        for (int i = 0; i < 4095; ++i) {
            buf2[i] = '0';
        }
        buf2[4095] = 0;
        readbuf[4095] = 0;
        if (read(fd, readbuf, 4095) != 4095) {
            return 1;
        }
        if (readbuf != buf2) {
            printf("Something went wrong: blocking mode.\n");
        }
        if (ioctl(fd, _IO(42, 0), NULL) != 0) {
            printf("IOCTL commands don't form from macros\n");
        }
        if (read(fd, buf, 6) != 1) {
            printf("non-blocking mode doesn't work as intended");
        }
        exit(0);
    }
}