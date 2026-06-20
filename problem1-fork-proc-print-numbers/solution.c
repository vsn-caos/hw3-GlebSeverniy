#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        return 1;
    }

    for (int current = 1; current <= n; ++current) {
        if (current == n) {
            printf("%d\n", current);
        } else {
            printf("%d ", current);
        }
        fflush(stdout);

        if (current == n) {
            return 0;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }

        if (pid > 0) {
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
                return 1;
            }
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
            return 1;
        }
    }

    return 0;
}
