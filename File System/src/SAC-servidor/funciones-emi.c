//
// Created by utnso on 10/11/19.
//

#include "funciones-emi.h"

//Funcion auxilliar para sac_mkdir
int escribir_dir(GBloque* disco, GFile* nodo, int puntero){
    int resultado = -1;
    nodo->size +=sizeof(puntero);
    int* bloque_memoria =(int*) (disco + nodo->GBloque[0]);

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

    if (existeNodo == -1 && nodoPadre != -1 && nodoPadre->estado == 2){

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
        t_list *listaBloquesLibres = buscarBloquesMemoriaLibres(1, disco, particion);

        //Inicializo ese bloque conseguido
        inicializarBloqueDirectorio(disco + *(int *) list_get(listaBloquesLibres, 0));

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

    int bloque;
    for(int i = 0; i < 1000; i++){
        if((bloque = nodoAEliminar->GBloque[i]) != -1){
            liberarBloqueMemoria( calcularCorreccion(bloque),disco,"../tools/disco.bin" );
            nodoAEliminar->GBloque[i] = -1;
        }
    }
    nodoAEliminar->size = 0;
    nodoAEliminar->nombre_archivo[0] = '\0';
    nodoAEliminar->estado = 0;

}

void eliminarDir(GFile* nodo, GBloque* disco){

    int* directorio = (int*) (disco + nodo->GBloque[0]);

    for (int i = 0; i < 1024; i++ ){

        if(directorio[i] != -1){
            GFile* tablaNodos = obtenerTablaNodos(disco);
            GFile* nodoAEliminar = tablaNodos + directorio[i];

            if(nodoAEliminar->estado == 2){
                eliminarDir(nodoAEliminar,disco);
            } else{
                eliminarNodo(nodoAEliminar, disco);
            }
        }

        directorio[i] = -1;
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
            int* directorioPadre = disco + nodoPadre->GBloque[0];
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

void main (){
    t_log* logger = log_create("formateo.log", "SAC", 0, LOG_LEVEL_TRACE);
//
//
//  formatear("../tools/disco.bin",logger);
//
//    //t_list * lista = buscarBloquesMemoriaLibres(1,disco, "../tools/disco.bin");
    GBloque* disco = mapParticion("../tools/disco.bin");
    GFile* carpetaRaiz = (GFile*) (disco+2);
//
    char* tuVieja = malloc(50);
    memcpy(tuVieja,"/Carpeta1",strlen("/Carpeta1")+1);
//    sac_mkdir(tuVieja);
//    memcpy(tuVieja,"/Carpeta2",strlen("/Carpeta2")+1);
//    sac_mkdir(tuVieja);
//    memcpy(tuVieja,"/Carpeta1/archivo1",strlen("/Carpeta1/archivo1")+1);
//    sac_mkdir(tuVieja);
//    memcpy(tuVieja,"/Carpeta1/Carpeta3",strlen("/Carpeta1/Carpeta3")+1);
//    sac_mkdir(tuVieja);
//    memcpy(tuVieja,"/Carpeta2/archivo1",strlen("/Carpeta2/archivo1")+1);
//    sac_mkdir(tuVieja);
//  sac_rmnod(tuVieja);
//  free(tuVieja);

    sac_rmdir(tuVieja);

    mostrarParticion("../tools/disco.bin");

    mostrarNodo(carpetaRaiz, disco);
    mostrarNodo(carpetaRaiz + 1, disco);
    mostrarNodo(carpetaRaiz + 2, disco);
    mostrarNodo(carpetaRaiz + 3, disco);
    mostrarNodo(carpetaRaiz + 4, disco);


    ;
    munmapParticion (disco, "../tools/disco.bin");
//
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
    free(tuVieja);
//    free(archivo);



}