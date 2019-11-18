//
// Created by utnso on 06/11/19.
//

#ifndef SAC_CLI_FUNCIONES_H
#define SAC_CLI_FUNCIONES_H

#include "tools/tools.h"
#include "structs.h"
#include <stdio.h>

int sac_mknod(char* path, mode_t mode, dev_t dev);

int sac_getattr(const char *path, struct stat *stbuf);

//int sac_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);


#endif //SAC_CLI_FUNCIONES_H
