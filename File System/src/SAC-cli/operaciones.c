#include "operaciones.h"
#include "fuse_example.h"


static int hello_getattr(const char *path, struct stat *stbuf) {
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));

    //Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, DEFAULT_FILE_PATH) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(DEFAULT_FILE_CONTENT);
    } else {
        res = -ENOENT;
    }
    printf("hello_getattr\n");
    return res;
}

static int example_fopen(const char *path, struct stat *stbuf){
    printf("example_fopen\n");
    return 0;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    // "." y ".." son entradas validas, la primera es una referencia al directorio donde estamos parados
    // y la segunda indica el directorio padre
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, DEFAULT_FILE_NAME, NULL, 0);
    printf("hello_readdir\n");
    return 0;
}

static int example_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int res;

    res = mknod(path, mode, rdev);
    if(res == -1)
        return -errno;

    printf("mknod\n");
    return 0;
}

static int example_utimens(const char *path, const struct timespec ts[2])
{
    int res;

    res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
    if (res == -1)
        return -errno;

    return 0;
}

static int example_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, DEFAULT_FILE_PATH) != 0)
        return -ENOENT;

    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    printf("example_open\n");
    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    size_t len;
    (void) fi;
    if (strcmp(path, DEFAULT_FILE_PATH) != 0)
        return -ENOENT;

    len = strlen(DEFAULT_FILE_CONTENT);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, DEFAULT_FILE_CONTENT + offset, size);
    } else
        size = 0;

    printf("hello_read\n");
    return 17;
}