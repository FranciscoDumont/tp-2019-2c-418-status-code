//
// Created by utnso on 06/11/19.
//

#include "funciones-rodri.h"


//int sac_mknod(char* path, mode_t mode, dev_t dev) { // mode y dev son los permisos del archivo
//    int current_node = 0;
//
//    //Traigo la particion a memoria
//    GBloque* disco = mapParticion(particion);
//    GFile* tablaNodos = obtenerTablaNodos(disco);
//
//    while(tablaNodos[current_node].estado != 0 && current_node < NOMBRE_ARCHIV_MAX) {
//        curent_node++;
//    }
//    if(current_node >= NOMBRE_ARCHIV_MAX){
//        return EDQUOT;
//    }
//
//    GFile* nodeToSet = GFile* tablaNodos + current_node;
//
//    strcpy((char*)nodeToSet->nombre_archivo, path + 1);
//    nodeToSet->size = 0;
//    nodeToSet->estado = 1;
//
//    //msync(disk,diskSize, MS_SYNC); -> para que es?
//    return 0;
//}

//int sac_getattr(const char *path, struct stat *stbuf) {
//    int res = 0;
//
//    //implementar el algoritmo de busqueda del archivo
//
//    memset(stbuf, 0, sizeof(struct stat));
//
//    //Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje
//
//    if (strcmp(path, "/") == 0) {
//        stbuf->st_mode = S_IFDIR | 0755;
//        stbuf->st_nlink = 2;
//    } else if (strcmp(path, DEFAULT_FILE_PATH) == 0) {
//        stbuf->st_mode = S_IFREG | 0444;
//        stbuf->st_nlink = 1;
//        stbuf->st_size = strlen(DEFAULT_FILE_CONTENT);
//    } else {
//        res = -ENOENT;
//    }
//    return res;
//}




int sac_read(const char *path, char *buf, size_t size, off_t offset){
    size_t len;
    //(void) fi;

    ////Divido el path en tokens o obtengo el nombre del archivo
//    t_list* pathDividido = dividirPath(path);
//    char* file_name = list_get(pathDividido,list_size(pathDividido)-1);

//    FILE* fd = fopen(file_name,"r");

    return 0;

//    len = obtenerTamanioArchivo(file_name);
//    if (offset < len) {
//        if (offset + size > len)
//            size = len - offset;
//        if (fseek(fd ,offset,SEEK_SET) == 0) {
//            memcpy(buf, fd, size);
//        } else
//            return -1;
//    } else
//        size = 0;
//
//    return size;
}


int main(){
    t_log* logger = log_create("formateo.log", "SAC", 0, LOG_LEVEL_TRACE);


    formatear("../tools/disco.bin",logger);

    //t_list * lista = buscarBloquesMemoriaLibres(1,disco, "../tools/disco.bin");
    GBloque* disco = mapParticion("../tools/disco.bin");
    GFile* carpetaRaiz = (GFile*) (disco+2);

//    char* tuVieja = malloc((strlen("/algo/No_anda_el_nombre_bien")));
//    memcpy(tuVieja,"/algo/No_anda_el_nombre_bien",strlen("/algo/No_anda_el_nombre_bien")+1);
//    sac_mkdir(tuVieja,0);
//    memcpy(tuVieja,"/algo/archivo_2",strlen("/algo/archivo_2")+1);
//    sac_mkdir(tuVieja,1);
//    memcpy(tuVieja,"/algo/archivo_3",strlen("/algo/archivo_3")+1);
//    sac_mkdir(tuVieja,1);

    mostrarParticion("../tools/disco.bin");

    mostrarNodo(carpetaRaiz, disco);
    mostrarNodo(carpetaRaiz + 1, disco);
    mostrarNodo(carpetaRaiz + 2, disco);
    mostrarNodo(carpetaRaiz + 3, disco);

    munmapParticion (disco, "../tools/disco.bin");
}