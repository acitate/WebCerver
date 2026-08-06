#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "filesystem.h"


FsStatus filesystem_read(const char *path, char **out_buf, size_t *out_len) {
    if (!path || !out_buf || !out_len) {
        return FS_ERR_UNEXPECTED;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        if (errno == ENOENT) {
            return FS_ERR_NOT_FOUND;
        }
        if (errno == EACCES) {
            return FS_ERR_PERMISSION_DENIED;
        }
        return FS_ERR_UNEXPECTED;
    }
    
    if (S_ISDIR(st.st_mode)) {
        return FS_ERR_IS_DIRECTORY;
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        if (errno == EACCES) {
            return FS_ERR_PERMISSION_DENIED;
        }
        return FS_ERR_UNEXPECTED;
    }

    size_t file_size = (size_t)st.st_size;
    char *buf = malloc(file_size + 1);
    if (!buf) {
        close(fd);
        return FS_ERR_UNEXPECTED;
    }

    ssize_t bytes_read = read(fd, buf, file_size);
    close(fd);

    if (bytes_read < 0) {
        free(buf);
        return FS_ERR_UNEXPECTED;
    }

    if ((size_t)bytes_read != file_size) {
        free(buf);
        return FS_ERR_UNEXPECTED;
    }

    buf[file_size] = '\0';
    *out_buf = buf;
    *out_len = file_size;
    return FS_OK;
}