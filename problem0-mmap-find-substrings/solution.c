#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifdef __MSYS__
#define LINE_END "\r\n"
#else
#define LINE_END "\n"
#endif

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <search_string>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *needle = argv[2];
    size_t needle_len = strlen(needle);

    if (needle_len == 0) {
        return 0;
    }

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    if (st.st_size <= 0 || (size_t)st.st_size < needle_len) {
        close(fd);
        return 0;
    }

    size_t file_len = (size_t)st.st_size;
    const char *data = mmap(NULL, file_len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    for (size_t pos = 0; pos + needle_len <= file_len; ++pos) {
        if (memcmp(data + pos, needle, needle_len) == 0) {
            printf("%zu%s", pos, LINE_END);
        }
    }

    if (munmap((void *)data, file_len) < 0) {
        perror("munmap");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
