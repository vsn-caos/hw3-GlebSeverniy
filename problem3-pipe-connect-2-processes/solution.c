#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void run_command(const char *command) {
    execlp("sh", "sh", "-c", command, (char *)NULL);
    perror("execlp");
    _exit(127);
}

static int status_to_code(int status) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <CMD1> <CMD2>\n", argv[0]);
        return 1;
    }

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t first = fork();
    if (first < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    if (first == 0) {
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            perror("dup2");
            _exit(127);
        }
        close(pipefd[0]);
        close(pipefd[1]);
        run_command(argv[1]);
    }

    pid_t second = fork();
    if (second < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(first, NULL, 0);
        return 1;
    }

    if (second == 0) {
        if (dup2(pipefd[0], STDIN_FILENO) < 0) {
            perror("dup2");
            _exit(127);
        }
        close(pipefd[0]);
        close(pipefd[1]);
        run_command(argv[2]);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    int first_status;
    int second_status;
    int rc = 0;
    if (waitpid(first, &first_status, 0) < 0) {
        perror("waitpid");
        rc = 1;
    }
    if (waitpid(second, &second_status, 0) < 0) {
        perror("waitpid");
        rc = 1;
    }

    if (rc != 0) {
        return rc;
    }
    return status_to_code(second_status);
}
