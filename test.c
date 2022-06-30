#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        int fd = open("/dev/mychardev-0", O_WRONLY);
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
        if (write(fd, buf2, 6) < 0) {
            exit(1);
        }
        exit(0);
    } else {
        int fd = open("/dev/mychardev-0", O_RDONLY);
        char buf[] = "mnklgh";
        if (read(fd, buf, 6) != 6) {
            printf("Something went wrong: read\n");
        }
        printf("%s\n", buf);
        char buf2[1024];
        if (read(fd, buf2, 4095) < 0) {
            printf("Something went wrong: read going over buffer size\n");
        }
        if (read(fd, buf, 6) < 0) {
            printf("Something went wrong: read going over buffer size\n");
        }
        printf("%s\n", buf);
        if (ioctl(fd, _IO(42, 0), NULL) != 0) {
            printf("IOCTL commands don't form from macros\n");
        }
        read(fd, buf, 2);
        printf("Non-blocking mode works as intended\n");
        int status;
        waitpid(pid, &status, 0);
        exit(WEXITSTATUS(status));
    }
}