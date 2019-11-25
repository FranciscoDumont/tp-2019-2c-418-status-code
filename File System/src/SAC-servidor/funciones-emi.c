//
// Created by utnso on 10/11/19.
//

#include "funciones-emi.h"

//Funcion auxilliar para sac_mkdir
int escribir_dir(GBloque* disco,GFile* nodo,int puntero){
    int cantidadArchivos = (nodo->size)/sizeof(int);
    nodo->size +=sizeof(puntero);

    //Desde el comienzo de la particion me muevo al bloque donde tengo guradado todo
    //Lo casteo a int para tener un array de puntero a bloques
    int* bloque_memoria =(int*) (disco + nodo->GBloque[0]);
    //Desde el array de int me muevo hasta el ultimo lugar ocupado
    *( bloque_memoria + cantidadArchivos ) = puntero;
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

    }else{
        list_destroy(pathDividido);
        munmapParticion(disco, "../tools/disco.bin");
        return -1;
    }
    list_destroy(pathDividido);
    munmapParticion(disco, "../tools/disco.bin");
    return 0;
}

int sac_rmdir(char* path){

}

void liberarBloqueMemoria(int bloque, GBloque* disco, char* nombreParticion){
    t_bitarray* bitmap = obtenerBitMap(nombreParticion, disco);

    bitarray_clean_bit(bitmap, bloque);

    bitarray_destroy(bitmap);

}
void eliminarNodoEnDirectorio(GFile* nodoPadre,int nodoAEliminar ){
    for(int i = 0;i < 1024; i++){
        if(nodoPadre->GBloque[i] == nodoAEliminar){
            nodoPadre->GBloque[i] = -1;

        }
    }
}

int sac_rmnod(char* path){
    t_list *pathDividido = dividirPath(path);
    int nro_nodo = buscarPath(pathDividido);

    t_list* listaSinElHijo = list_take(pathDividido, list_size(pathDividido) - 1);
    int nro_nodo_padre = buscarPath(listaSinElHijo);
    list_destroy(listaSinElHijo);

    GBloque* disco = mapParticion("../tools/disco.bin");
    GFile* tablaNodos = obtenerTablaNodos(disco);
    GFile* nodoAEliminar = tablaNodos + nro_nodo;

    int bloque;
    for(int i = 0; i < 1000; i++){
        if(bloque = nodoAEliminar->GBloque[i] != -1){
            liberarBloqueMemoria(bloque,disco,"../tools/disco.bin" );
            nodoAEliminar->GBloque[i] = -1;
        }
    }
    nodoAEliminar->nombre_archivo[0] = '\0';
    eliminarNodoEnDirectorio(tablaNodos + nro_nodo_padre, nro_nodo);
    nodoAEliminar->estado = 0;

    munmapParticion(disco,"../tools/disco.bin");
}

void main (){
//    t_log* logger = log_create("formateo.log", "SAC", 0, LOG_LEVEL_TRACE);
//
//
//   formatear("../tools/disco.bin",logger);

    //t_list * lista = buscarBloquesMemoriaLibres(1,disco, "../tools/disco.bin");
    GBloque* disco = mapParticion("../tools/disco.bin");
    GFile* carpetaRaiz = (GFile*) (disco+2);
//
    char* tuVieja = malloc(50);
//   memcpy(tuVieja,"/Carpeta1",strlen("/Carpeta1")+1);
//    sac_mkdir(tuVieja);
//    memcpy(tuVieja,"/Carpeta2",strlen("/Carpeta2")+1);
//    sac_mkdir(tuVieja);
    memcpy(tuVieja,"/Carpeta1/archivo1",strlen("/Carpeta1/archivo1")+1);
//    sac_mkdir(tuVieja);
//    memcpy(tuVieja,"/Carpeta24/archivo1",strlen("/Carpeta24/archivo1")+1);
//    sac_mkdir(tuVieja);
    sac_rmnod(tuVieja);

    mostrarParticion("../tools/disco.bin");

    mostrarNodo(carpetaRaiz, disco);
    mostrarNodo(carpetaRaiz + 1, disco);
    mostrarNodo(carpetaRaiz + 2, disco);
    mostrarNodo(carpetaRaiz + 3, disco);
    mostrarNodo(carpetaRaiz + 4, disco);


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
//    free(tuVieja);
//    free(archivo);



}