//
// Created by utnso on 06/11/19.
//

#include "funciones.h"

GFile* buscar_archivo(char* path){


}

extern file_t* nodeTable;

int sac_mknod(char* path, mode_t mode, dev_t dev) { // mode y dev son los permisos del archivo
    int current_node = 0;

    while(nodeTable[current_node].estado != 0 && current_node < NOMBRE_ARCHIV_MAX) {
        curent_node++;
    }
    if(current_node >= NOMBRE_ARCHIV_MAX){
        return EDQUOT;
    }

    GFile* nodeToSet = GFile* nodeTable + current_node;

    strcpy((char*)nodeToSet->nombre_archivo, path + 1);
    nodeToSet->size = 0;
    nodeToSet->estado = 1;

    //msync(disk,diskSize, MS_SYNC); -> para que es?
    return 0;
}

int sac_getattr(const char *path, struct stat *stbuf) {
    int res = 0;

    //implementar el algoritmo de busqueda del archivo

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
    return res;
}