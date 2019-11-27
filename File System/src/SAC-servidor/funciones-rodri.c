//
// Created by utnso on 06/11/19.
//

#include "funciones-rodri.h"


int obtenerNodoPadre(char* path){
    int nodoPadre = 0;
    ////Divido el path y obtengo la lista menos el ultimo elemento
    t_list* pathDividido = dividirPath(path);

    if(list_size(pathDividido) < 3){ ////esta en la carpeta raiz
        list_destroy(pathDividido);
        return nodoPadre;
    }else{

        t_list *pathModificado = list_take(pathDividido, list_size(pathDividido) - 1);
        nodoPadre = buscarPath(pathModificado);
        list_destroy(pathModificado);
        list_destroy(pathDividido);
        return nodoPadre;

        }
}


int esArchivoODirectorio(char* path){

    ////Divido el path y obtengo el nombre
    t_list* pathDividido = dividirPath(path);
    char* file_name = list_get(pathDividido,list_size(path)-1);

    ////Verifico si tiene un punto en la cadena
    char* token = strtok(file_name, ".");

    if(token != NULL){ ////Es un archivo porque tiene una extension
        return 1;
    }else {
        return 2; ////Si es null quiere decir que el file_name no tiene extension por ende es un directorio
    }
}


int sac_mknod(char* path, mode_t mode, dev_t dev){ // mode y dev son los permisos del archivo
    int current_node = 0;
    char* fecha_actual = obtenerFechaActual();
    char* copiaPath = (char *)malloc( 75 * sizeof(char));
    int estado;
    int nodoPadre;

    //Traigo la particion a memoria
    char* particion =  "../tools/disco.bin";
    GBloque* disco = mapParticion(particion);
    GFile* tablaNodos = obtenerTablaNodos(disco);

    ////Obtengo un nodo vacio

    current_node = obtenerNodoLibre(tablaNodos);

    if(current_node >= NOMBRE_ARCHIV_MAX){
        return EDQUOT;
    }

    ////Me posiciono en el nodo que quiero crear con aritmetica de punteros
    GFile* nodeToSet = tablaNodos + current_node;

    nodoPadre = obtenerNodoPadre(path);

    t_list* listaBloquesLibres = buscarBloquesMemoriaLibres(1,disco, particion);

    strcpy((char*)nodeToSet -> nombre_archivo, path + 1);
    nodeToSet->size = 0;
    nodeToSet->fecha_creacion =  atoi(fecha_actual);
    nodeToSet->fecha_modificacion = atoi(fecha_actual);
    nodeToSet -> estado = 1;
    nodeToSet->ptr_bloque_padre = nodoPadre;
    nodeToSet->GBloque[0] = *(int*) list_get(listaBloquesLibres,0);

//    int tamanio_disco = obtenerTamanioArchivo(particion);
//
//    msync(disco,tamanio_disco, MS_SYNC);
    free(copiaPath);
    free(fecha_actual);
    list_destroy(listaBloquesLibres);
    munmapParticion(disco, particion);
    return 0;
}

int sac_getattr(const char *path, struct stat *stbuf){
    int res = 0;
    int current_node = 0;
    GFile* nodo;

    t_list* pathDividido = dividirPath(path);
    char* nombre_archivo = list_get(pathDividido,list_size(pathDividido)-1);

    char* particion =  "../tools/disco.bin";
    GBloque* disco = mapParticion(particion);
    GFile* tablaNodos = obtenerTablaNodos(disco);

    while(strcmp(tablaNodos[current_node].nombre_archivo, nombre_archivo) != 0){
        current_node ++;
    }

    nodo = tablaNodos + current_node;


    memset(stbuf, 0, sizeof(struct stat));

    if(nodo -> estado == 2) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        stbuf->st_mtime = nodo->fecha_modificacion;
    } else if(nodo -> estado == 1){
        stbuf -> st_mode = S_IFREG | 0444;
        stbuf -> st_nlink = 1;
        stbuf -> st_size = obtenerTamanioArchivo(nombre_archivo);
        stbuf->st_mtime = nodo->fecha_modificacion;
    } else {
        res = -ENOENT;
    }
    return res;
}



int sac_read(const char *path, char *buf, size_t size, off_t offset){
    size_t len;

    //Divido el path en tokens o obtengo el nombre del archivo
    t_list* pathDividido = dividirPath(path);
    char* file_name = list_get(pathDividido,list_size(pathDividido)-1);

    FILE* fd = fopen(file_name,"r");


    len = obtenerTamanioArchivo(file_name);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        if (fseek(fd ,offset,SEEK_SET) == 0) {
            memcpy(buf, fd, size);
        } else
            return -1;
    } else
        size = 0;

    return size;
}


int main(){
    t_log* logger = log_create("formateo.log", "SAC", 0, LOG_LEVEL_TRACE);


    formatear("../tools/disco.bin",logger);

    //t_list * lista = buscarBloquesMemoriaLibres(1,disco, "../tools/disco.bin");
    GBloque* disco = mapParticion("../tools/disco.bin");
    GFile* carpetaRaiz = (GFile*) (disco+2);
    mostrarParticion("../tools/disco.bin");

    char* buffer = malloc(50);

    memcpy(buffer,"/archivo1",strlen("/archivo1")+1);
    sac_mknod(buffer, NULL, NULL);
    memcpy(buffer,"/archivo1/archivo2",strlen("/archivo1/archivo2")+1);
    sac_mknod(buffer, NULL, NULL);
//    memcpy(buffer,"/archivo1/archivo2/archivo3",strlen("/archivo1/archivo2/archivo3")+1);
//    sac_mknod(buffer, NULL, NULL);



    mostrarParticion("../tools/disco.bin");

    mostrarNodo(carpetaRaiz, disco);
    mostrarNodo(carpetaRaiz, disco + 2);
    mostrarNodo(carpetaRaiz, disco + 3);

    munmapParticion (disco, "../tools/disco.bin");


}