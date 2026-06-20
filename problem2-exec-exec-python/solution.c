#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    size_t capacity = 128;
    size_t length = 0;
    char *expr = malloc(capacity);
    if (expr == NULL) {
        perror("malloc");
        return 1;
    }

    for (;;) {
        char buffer[256];
        ssize_t bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes < 0) {
            perror("read");
            free(expr);
            return 1;
        }
        if (bytes == 0) {
            break;
        }

        if (length + (size_t)bytes + 1 > capacity) {
            while (length + (size_t)bytes + 1 > capacity) {
                capacity *= 2;
            }
            char *new_expr = realloc(expr, capacity);
            if (new_expr == NULL) {
                perror("realloc");
                free(expr);
                return 1;
            }
            expr = new_expr;
        }

        memcpy(expr + length, buffer, (size_t)bytes);
        length += (size_t)bytes;
    }

    while (length > 0 && (expr[length - 1] == '\n' || expr[length - 1] == '\r')) {
        --length;
    }
    expr[length] = '\0';

    const char prefix[] = "print(";
    const char suffix[] = ")";
    size_t code_len = strlen(prefix) + length + strlen(suffix);
    char *code = malloc(code_len + 1);
    if (code == NULL) {
        perror("malloc");
        free(expr);
        return 1;
    }

    memcpy(code, prefix, strlen(prefix));
    memcpy(code + strlen(prefix), expr, length);
    memcpy(code + strlen(prefix) + length, suffix, strlen(suffix) + 1);

    execlp("python3", "python3", "-c", code, (char *)NULL);
    perror("execlp");

    free(code);
    free(expr);
    return 1;
}
