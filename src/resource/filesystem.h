#ifndef FILESYSTEM_H
#define FILESYSTEM_H

typedef enum {
    FS_OK = 0,
    FS_ERR_NOT_FOUND = 1,
    FS_ERR_PERMISSION_DENIED = 2,
    FS_ERR_IS_DIRECTORY = 3,
    FS_ERR_UNEXPECTED = 4
} FsStatus;

FsStatus filesystem_read(const char *path, char **out_buf, size_t *out_len);

#endif