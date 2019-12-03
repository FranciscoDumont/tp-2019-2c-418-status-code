//
// Created by utnso on 10/11/19.
//

#include "funciones-emi.h"
#include <dirent.h>
#include <stdlib.h>

typedef struct def{
    int nro_nodo;
    int nro_byts;
    char nombre[71];
}Dir;

int* obtenerDirectorio(int nodo, GBloque* disco){
    GFile* tablaNodos = obtenerTablaNodos(disco);
    GFile* nodoAObtener = tablaNodos + nodo;

    int* punteroIndirecto = disco + nodoAObtener->GBloque[0];
    int* directorio = disco + punteroIndirecto[0];

    return directorio;
}

t_list* sac_readdir (char* path){
    t_list* respuesta = list_create();
    t_list* pathDividido = dividirPath(path);
    int nro_nodo = buscarPath (pathDividido);
    GBloque* disco = mapParticion("../tools/disco.bin");
    GFile* tablaNodos = obtenerTablaNodos(disco);
    GFile* nodo = tablaNodos + nro_nodo;
    int* directorio = obtenerDirectorio(nro_nodo, disco);
    for ( int i = 0; i < 1024; i++){
        if(directorio[i]!=-1){
            Dir* dirAEnviar = malloc(sizeof(Dir));
            int nro_nodo_hijo = directorio[i];
            GFile* nodo_hijo = tablaNodos + nro_nodo_hijo;
            memcpy(dirAEnviar->nombre, nodo_hijo->nombre_archivo, strlen(nodo_hijo->nombre_archivo)+1);
            dirAEnviar->nro_nodo=nro_nodo_hijo;
            dirAEnviar->nro_byts = strlen(nodo_hijo->nombre_archivo) + sizeof(int);
            list_add(respuesta,dirAEnviar);
        }
    }

    return respuesta;
}
void mostrarCarpeta(char* path){
   t_list* directorio = sac_readdir (path);

   for(int i = 0; i < list_size(directorio) ; i++){
       Dir* a =  list_get(directorio,i);
       printf("Nombre:%s\t Numero nodo:%d\t Numero Byts: %d\n",a->nombre, a->nro_nodo, a->nro_byts);
   }
}


//Funcion auxilliar para sac_mkdir
int escribir_dir(GBloque* disco, GFile* nodo, int puntero){
    int resultado = -1;
    nodo->size +=sizeof(puntero);
    int* bloque_memoria_indirecto =(int*) (disco + nodo->GBloque[0]);
    int* bloque_memoria = (int*) (disco + bloque_memoria_indirecto[0]);
    for(int i = 0; i < 1024; i++){
        if(bloque_memoria[i]==-1){
            bloque_memoria[i] = puntero;
            resultado = 0;
            break;
        }
    }

    return resultado;
}

//Crea una carpeta en disco
//Retorna -1 si es error sino 0
int sac_mkdir(char* path ) {
    t_list *pathDividido = dividirPath(path);

    //Esto porahi hay que sacarlo
    char *particion = "../tools/disco.bin";

    int existeNodo = buscarPath(pathDividido);
    char *nombre_dir = list_get(pathDividido, list_size(pathDividido)-1);
    t_list* listaSinElHijo = list_take(pathDividido, list_size(pathDividido) - 1);
    int nro_nodoPadre = buscarPath(listaSinElHijo);
    list_destroy(listaSinElHijo);

    //Traigo la particion a memoria
    GBloque *disco = mapParticion(particion);
    GFile *tablaNodos = obtenerTablaNodos(disco);
    GFile *nodoPadre = tablaNodos+ nro_nodoPadre;

    if (existeNodo == -1 && nro_nodoPadre != -1 && nodoPadre->estado == 2){

        char *fecha_actual = obtenerFechaActual();

        //Obtengo en nro de nodo libre donde voy a poner mi directorio nuevo
        int nro_nodoLibre = obtenerNodoLibre(tablaNodos);

        //Me fijo si hay nodos libres
        if (nro_nodoLibre == -1) {
            munmapParticion(disco, "../tools/disco.bin");
            return -1;
        }

        //Obtengo el puntero a ese nodo  y su bloque para el directorio
        GFile *nodoLibre = tablaNodos + nro_nodoLibre;
        t_list *listaBloquesLibres = buscarBloquesMemoriaLibres(2, disco, particion);

        //Inicializo ese bloque conseguido
        inicializarBloqueDirectorio(disco + *(int *) list_get(listaBloquesLibres, 0));
        inicializarBloqueDirectorio(disco + *(int *) list_get(listaBloquesLibres, 1));

        //Devuelve error si no hay bloques libres
        if (list_is_empty(listaBloquesLibres)) {
            list_destroy(pathDividido);
            munmapParticion(disco, "../tools/disco.bin");
            return -1;
        }

        memcpy((nodoLibre->nombre_archivo), nombre_dir, strlen(nombre_dir) + 1);
        nodoLibre->estado = 2;
        nodoLibre->fecha_creacion = atoi(fecha_actual);
        nodoLibre->fecha_modificacion = atoi(fecha_actual);
        nodoLibre->ptr_bloque_padre = nro_nodoPadre;
        nodoLibre->GBloque[0] = *(int *) list_get(listaBloquesLibres, 0);
        int* punteroIndirecto = (int*)(disco + *(int *) list_get(listaBloquesLibres, 0));
        punteroIndirecto[0] = *(int *) list_get(listaBloquesLibres, 1);
        nodoLibre->size = 0;


        //Escribo en el nodo padre la direccion del nuevo nodo
        escribir_dir(disco, tablaNodos + nro_nodoPadre, nro_nodoLibre);
        free(fecha_actual);
        free(list_get(listaBloquesLibres,0));
        list_destroy(listaBloquesLibres);

    }else{
        list_destroy(pathDividido);
        munmapParticion(disco, "../tools/disco.bin");
        return -1;
    }
    list_destroy(pathDividido);
    munmapParticion(disco, "../tools/disco.bin");
    return 0;
}

void eliminarNodo(GFile* nodoAEliminar, GBloque* disco){

    for(int i = 0; i < 1000; i++){
        int bloque_indirecto = nodoAEliminar->GBloque[i];
        int* bloque = (int*) (disco + bloque_indirecto);
        if(bloque_indirecto != -1){
            for(int j = 0; j < 1024; j++){
                if(bloque[j] != -1){
                    liberarBloqueMemoria( calcularCorreccion(bloque[j]),disco,"../tools/disco.bin" );
                }
            }
            nodoAEliminar->GBloque[i] = -1;
        }
        liberarBloqueMemoria( calcularCorreccion(bloque_indirecto),disco,"../tools/disco.bin" );
    }
    nodoAEliminar->size = 0;
    nodoAEliminar->nombre_archivo[0] = '\0';
    nodoAEliminar->estado = 0;

}

void eliminarDir(GFile* nodo, GBloque* disco){

    int* directorio = (int*) (disco + nodo->GBloque[0]);
    int* puntero_simple = (int *) (disco + directorio[0]);

    for (int i = 0; i < 1024; i++ ){

        if(puntero_simple[i] != -1){
            GFile* tablaNodos = obtenerTablaNodos(disco);
            GFile* nodoAEliminar = tablaNodos + puntero_simple[i];

            if(nodoAEliminar->estado == 2){
                eliminarDir(nodoAEliminar,disco);
            } else{
                eliminarNodo(nodoAEliminar, disco);
            }
        }

    }

    eliminarNodo(nodo, disco);
}

int sac_rmdir(char* path){
    t_list *pathDividido = dividirPath(path);
    int nro_nodo = buscarPath(pathDividido);

    list_destroy(pathDividido);
    int resultado = -1;
    if(nro_nodo != -1){
        GBloque* disco = mapParticion("../tools/disco.bin");
        GFile* tablaNodos = obtenerTablaNodos(disco);
        GFile* nodoAEliminar = tablaNodos + nro_nodo;

        if(nodoAEliminar->estado == 2){
            eliminarDir(nodoAEliminar, disco);
            GFile* nodoPadre = tablaNodos + nodoAEliminar->ptr_bloque_padre;
            int* punteroIndirectoPadre = disco + nodoPadre->GBloque[0];
            int* directorioPadre = disco + punteroIndirectoPadre[0];
            eliminarNodoEnDirectorio( directorioPadre , nro_nodo);
            resultado = 0;
        }

        munmapParticion(disco,"../tools/disco.bin");
    }

    return resultado;

}

void eliminarNodoEnDirectorio(int* bloqueDirectorio, int nodoAEliminar ){

    for(int i = 0;i < 1024; i++){
        if(bloqueDirectorio[i]  == nodoAEliminar){
            bloqueDirectorio[i] = -1;
        }
    }
}

int sac_rmnod(char* path){
    t_list *pathDividido = dividirPath(path);
    int nro_nodo = buscarPath(pathDividido);

    t_list* listaSinElHijo = list_take(pathDividido, list_size(pathDividido) - 1);
    int nro_nodo_padre = buscarPath(listaSinElHijo);

    //Destruyo las listas del path
    list_destroy(listaSinElHijo);
    list_destroy(pathDividido);

    //Empiezo a trabajar con el disco
    GBloque* disco = mapParticion("../tools/disco.bin");
    GFile* tablaNodos = obtenerTablaNodos(disco);
    GFile* nodoAEliminar = tablaNodos + nro_nodo;

    int resultado = -1;
    if(nodoAEliminar->estado == 1){

        eliminarNodo(nodoAEliminar,disco);
        GFile* nodoPadre = tablaNodos + nro_nodo_padre;
        nodoPadre->size -= sizeof(int);
        GBloque* bloqueDir = disco+nodoPadre->GBloque[0];
        eliminarNodoEnDirectorio((int*) bloqueDir, nro_nodo);
        resultado = 0;
    }

    return resultado;
    munmapParticion(disco,"../tools/disco.bin");
}


typedef struct elementoOpen{
    int nro_nodo;
    char* path;
    uint8_t estado;
    int cantidad_aperturas;
}elementoOpen;

t_list* tablaArchivosAbiertos;

void incializarTabla (){
    tablaArchivosAbiertos = list_create();
}

char* intToCharPointer(int nro){
    char* fd = malloc(5);
    sprintf(fd, "%d", nro);;

    return fd;
}

/*
 * Busca si existe la fucion en la tabla de archivos
 * @return el indice en la tabla de archivos, -1 si no existe
 * @Parametros nro_nod
 */

int existeEnTablaDeArchivos(int nro_nodo){
    int retorno = -1;

    for(int i = 0; i < list_size(tablaArchivosAbiertos); i++){
        elementoOpen* elemento = list_get(tablaArchivosAbiertos,i);
        if(elemento->nro_nodo == nro_nodo){
            retorno = i;
            break;
        }
    }

    return retorno;
}

/*
 * Funcion para agregar a la tabla de archivos abiertos
 * @parametros = path del archivo
 * @return = file descriptor
 */

int sac_open (char* path) {
    char* pathElemetno = malloc(strlen(path) + 1);
    memcpy(pathElemetno, path, strlen(path) + 1);
    char* pathDividido = dividirPath(path);
    int nro_nodo = buscarPath(pathDividido);

    elementoOpen *elemento = malloc(sizeof(elementoOpen));

    if (existeEnTablaDeArchivos(nro_nodo) == -1) {
        elemento->path = pathElemetno;
        t_list *pathDividido = dividirPath(path);
        elemento->nro_nodo = nro_nodo;
        elemento->estado = 0;
        elemento->cantidad_aperturas = 1;

        list_add(tablaArchivosAbiertos, (void *) elemento);
    }else{
        int index = existeEnTablaDeArchivos(nro_nodo);
        elemento = list_get(tablaArchivosAbiertos,index);
        elemento->cantidad_aperturas ++;
    }

    return nro_nodo;
}

int sac_close(int fd){
    int index = existeEnTablaDeArchivos(fd);

    if(index != -1){
        elementoOpen* elemento  = list_get(tablaArchivosAbiertos, index);
        if(elemento->cantidad_aperturas == 1){
            list_remove(tablaArchivosAbiertos, index);

            free(elemento->path);
            free(elemento);
        }else{
            elemento->cantidad_aperturas -= 1;
        }
    }

    return index;

}
void main (){
    t_log* logger = log_create("formateo.log", "SAC", 0, LOG_LEVEL_TRACE);

    formatear("../tools/disco.bin",logger);

    char* tuVieja = malloc(50);
    memcpy(tuVieja,"/Carpeta1",strlen("/Carpeta1")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta2",strlen("/Carpeta2")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta1/archivo1",strlen("/Carpeta1/archivo1")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta1/Carpeta3",strlen("/Carpeta1/Carpeta3")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta1/algo",strlen("/Carpeta1/algo")+1);
    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta2/archivo1",strlen("/Carpeta2/archivo1")+1);
    sac_mkdir(tuVieja);

    memcpy(tuVieja,"/Carpeta1",strlen("/Carpeta1")+1);
//    mostrarCarpeta(tuVieja);

    //  free(tuVieja);

//    sac_rmdir(tuVieja);
    GBloque* disco = mapParticion("../tools/disco.bin");
    GFile* carpetaRaiz = (GFile*) (disco+2);
    incializarTabla();
    mostrarParticion("../tools/disco.bin");
    memcpy(tuVieja,"/Carpeta2/archivo1",strlen("/Carpeta2/archivo1")+1);
    //printf("\nDescriptor de archivo: %d\n", sac_open(tuVieja));
    sac_open(tuVieja);

    memcpy(tuVieja,"/Carpeta1/algo",strlen("/Carpeta1/algo")+1);
    //printf("\nDescriptor de archivo: %d\n", sac_open(tuVieja));
    sac_open(tuVieja);
    memcpy(tuVieja,"/Carpeta1/algo",strlen("/Carpeta1/algo")+1);
    sac_open(tuVieja);
    memcpy(tuVieja,"/Carpeta1/algo",strlen("/Carpeta1/algo")+1);
    nro_nodo = sac_open(tuVieja);


    for(int i = 0; i < list_size(tablaArchivosAbiertos); i++ ){
        elementoOpen* el = list_get(tablaArchivosAbiertos, i);
        printf("\nEstado: %d, path: %s cantidad aperturas: %d\n", el->estado, el->path, el->cantidad_aperturas);
    }

    sac_close(nro_nodo);


    for(int j = 0; j < list_size(tablaArchivosAbiertos); j++ ){
        elementoOpen* el = list_get(tablaArchivosAbiertos, j);
        printf("\nEstado: %d, path: %s cantidad aperturas: %d\n", el->estado, el->path, el->cantidad_aperturas);
    }


    //
//    mostrarNodo(carpetaRaiz, disco);
//    mostrarNodo(carpetaRaiz + 1, disco);
//    mostrarNodo(carpetaRaiz + 2, disco);
//    mostrarNodo(carpetaRaiz + 3, disco);
//    mostrarNodo(carpetaRaiz + 4, disco);
//    mostrarNodo(carpetaRaiz + 5, disco);
//
//
//    ;
//    munmapParticion (disco, "../tools/disco.bin");
////
//    memcpy(tuVieja,"/",strlen("/")+1);
//
//    t_list* path = dividirPath(tuVieja);
//    printf("%d", buscarPath(path));
//    list_destroy(path);
//    free(tuVieja);
    //printf("\nestado:%d   bloque usado: %d", carpetaRaiz->estado,carpetaRaiz->GBloque[0]);

//    char* archivo= malloc(50);
//    memcpy(archivo,"archivo1",strlen("archivo1")+1);
//
//    t_list* posibles_nodos = hallar_posibles_padres(archivo);
//
//    for(int i= 0; i < list_size(posibles_nodos); i++){
//        printf("%d",list_get(posibles_nodos,i));
//    }
//
    log_destroy(logger);
//    free(tuVieja);
//    free(archivo);



}