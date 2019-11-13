//
// Created by utnso on 10/11/19.
//

#include "funciones-emi.h"

/*Dividi el path en sus directorios y los devuelve en froma
 *de lista.
 *Comprobar caso que manden path sin / y el caso que este solo /
 * Si le pasas una constante de string te llora
*/

/* Si tenes el puntero al bloque de un directorio
 * esta funcion te va a devolver el puntero
 * al nodo que buscas
 */
//Ver de cambiarle el nombre


GFile* hallar_padre(GBloque* bloque, char* nombrePadre){
   int* punteroArchivo = (int*) bloque;
    GFile* nodo;

    //Si igual tener que ver si se llego al final del artchivo o algo asi
    //Le pondria eof al final de los puntero o menos 1 no se
    for(int i = 0; i < BLOQUE_TAMANIO/4 && *punteroArchivo != EOF;i++){
//        nodo =buscarNodo(punteroArchivo);

        if(strcmp(nodo->nombre_archvio,nombrePadre) == 0){
            return nodo;
        }
        punteroArchivo++;
    }
    return -1;
}

//GFile* buscarNodo(GFile* tablaNodos, int numeroNodo){
//    return tablaNodos + numeroNodo;
//}

/* Divide el path ssegun el token que le pases
 * *IMPORTANTE* Liberar el path despues de usar el token, no antes
 */

t_list* dividirPath(char* path){
    char* token = strtok(path, "/");
    t_list* pathDividido = list_create();

    while(token != NULL){
        list_add(pathDividido, token);
        token = strtok(NULL, "/");
    }
    return pathDividido;
}

////Probar Dudo que funcione
//void buscarPath(t_list* pathDividido){
//   //Inicaliizo las variables
//   t_list* nodosCandidatos = list_create();
//   GFile* nodo;
//
//   //Busco en todos los nodos si hay algunos que tengan ese nombre
//   for(int nro_nodo = 0 ; nro_nodo < 1024; nro_nodo++){
//       GFile* nodo = buscarNodo();
//       //Obtengo el nombre del archivo que es el ultimo en tokenizar
//       char*nombreArchivo = (char*) list_get(pathDividido, list_size(pathDividido)-1);
//
//        //Si comparo los nombres y es verdadero los agrego a la lista de nodos candidatos
//       if(strcmp(nodo->nombre_archvio,nombreArchivo) == 0){
//           list_add(nodosCandidatos, nodo);
//       }
//   }
//
//   nodo = list_get(nodosCandidatos,0);
//   //Recorres toda la familia de los nodos candidatos para ver que coincidan
//   for(int i = 0; i < list_size(nodosCandidatos) && nodo->ptr_bloque_padre == 0; i++){
//      bool noEraPadre = 0;
//       //Hallo el nodo padre con el puntero al bloque donde esta alojado
//       GFile* nodoPadre = hallar_padre(nodo->ptr_bloque_padre,list_get(pathDividido,1));
//
//       //Recorre toda la familia de ese nodo, j = 1 porque me salteo el nombre
//       for(int j = 1; nodoPadre->ptr_bloque_padre == 0 && j <= list_size(pathDividido); j++){
//
//           //Si hallar padre devuelve menos uno es que no se encontro un nodo en el bloque
//           // que esta el padre con ese nombre
//           if(nodoPadre == -1){
//               noEraPadre = 1;
//               break;
//           }
//
//           GFile* nodoPadre = hallar_padre(nodoPadre->ptr_bloque_padre,list_get(pathDividido,list_size(pathDividido)-j-1));
//       }
//
//       //Saco de la lista al nodo que no llego a la raiz
//       if(noEraPadre) {
//           list_remove(nodosCandidatos, i);
//       }
//       nodo = list_get(nodosCandidatos,i+1);
//
//   }
//
//   return list_is_empty(nodosCandidatos)?-1:list_get(nodosCandidatos,0);
//}

GFile* obtenerTablaNodos(GBloque* comienzoParticion){
    GHeader* header = (GHeader*)comienzoParticion;

    //Y saco del header el tamanio del bitmap
    //Corro entonces el puntero 1 bloque mas los bloques que ocupa el bitMap
    return (GFile *) header + 1 + header->bitMap_tamanio;
}

int obtenerBitMapSize(char* particion){
    int disco_size = obtenerTamanioArchivo(particion);
    return obtenerCantidadBytsBitmap(disco_size);
}

t_bitarray* obtenerBitMap(char* particion, GBloque* disco){

    int tamanioBitmap = obtenerBitMapSize(particion);

    return bitarray_create((char *)(disco + 1), tamanioBitmap);
}


int obtenerNodoLibre (GFile* comienzoTabla){
    for(int i = 0; i  < CANTIDAD_ARCHIVOS_MAX; i++ ) {
        if ((comienzoTabla + i)->estado == 0) {
            return i;

        }
    }
    return -1;
}

int escribir_dir(GBloque* disco,GFile* nodo,int puntero){
    int cantidadArchivos = (nodo->size)/sizeof(int);
    nodo->size +=sizeof(puntero);

    //Desde el comienzo de la particion me muevo al bloque donde tengo guradado todo
    //Lo casteo a int para tener un array de puntero a bloques
    int* bloque_memoria =(int*) (disco + nodo->GBloque[0]);
    //Desde el array de int me muevo hasta el ultimo lugar ocupado
    *( bloque_memoria + cantidadArchivos ) = puntero;
}

int sac_mkdir(char* path, int nro_nodoPadre){

    //Esto porahi hay que sacarlo
    char* particion =  "../tools/disco.bin";

    //Obtengo el path dividido para la funcion de busqueda
    t_list* pathDividido = dividirPath(path);
    char* nombre_dir = list_get(pathDividido,list_size(pathDividido)-1);

    //Traigo la particion a memoria
    GBloque* disco = mapParticion(particion);
    GFile* tablaNodos = obtenerTablaNodos(disco);

    //Obtengo en nro de nodo libre donde voy a poner mi directorio nuevo
    int nro_nodoLibre = obtenerNodoLibre(tablaNodos);

    //Me fijo si hay nodos libres
    if(nro_nodoLibre == -1){
        return -1;
    }

    //Obtengo el puntero a ese nodo  y su bloque para el directorio
    GFile* nodoLibre = tablaNodos + nro_nodoLibre;
    t_list* listaBloquesLibres = buscarBloquesMemoriaLibres(1, disco, particion);

    //Inicializo ese bloque conseguido
    inicializarBloqueDirectorio(disco + *(int*)list_get(listaBloquesLibres,0));

    //Devuelve error si no hay bloques libres
    if(list_is_empty(listaBloquesLibres)){
        return -1;
    }

    memcpy((nodoLibre->nombre_archvio), nombre_dir, strlen(nombre_dir)+1);
    nodoLibre->estado = 2;
    nodoLibre->fecha_creacion = nodoLibre->fecha_modificacion = obtenerFechaActual();
    nodoLibre->ptr_bloque_padre = nro_nodoPadre;
    nodoLibre->GBloque[0] = *(int*)list_get(listaBloquesLibres,0);
    nodoLibre->size = 0;


    //Escribo en el nodo padre la direccion del nuevo nodo
    escribir_dir(disco,tablaNodos + nro_nodoPadre, nro_nodoLibre);

    munmapParticion (disco, "../tools/disco.bin");

    return 0;
}

void main (){
    t_log* logger = log_create("formateo.log", "SAC", 0, LOG_LEVEL_TRACE);


    formatear("../tools/disco.bin",logger);

    //t_list * lista = buscarBloquesMemoriaLibres(1,disco, "../tools/disco.bin");
    GBloque* disco = mapParticion("../tools/disco.bin");
    GFile* carpetaRaiz = (GFile*) (disco+2);

    char* tuVieja = malloc((strlen("/algo/No_anda_el_nombre_bien")));
    memcpy(tuVieja,"/algo/No_anda_el_nombre_bien",strlen("/algo/No_anda_el_nombre_bien")+1);
    sac_mkdir(tuVieja,0);
    memcpy(tuVieja,"/algo/archivo_2",strlen("/algo/archivo_2")+1);
    sac_mkdir(tuVieja,1);
    memcpy(tuVieja,"/algo/archivo_3",strlen("/algo/archivo_3")+1);
    sac_mkdir(tuVieja,1);

    mostrarParticion("../tools/disco.bin");

    mostrarNodo(carpetaRaiz, disco);
    mostrarNodo(carpetaRaiz + 1, disco);
    mostrarNodo(carpetaRaiz + 2, disco);
    mostrarNodo(carpetaRaiz + 3, disco);

    munmapParticion (disco, "../tools/disco.bin");



    //printf("\n%d\n", *(int*)list_get(lista,0));
    //printf("\nestado:%d   bloque usado: %d", carpetaRaiz->estado,carpetaRaiz->GBloque[0]);

}