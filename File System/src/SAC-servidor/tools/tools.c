#include "tools.h"



/*****************************
 **** Funciones auxiliares****
 *****************************
 */

t_list * buscarBloquesMemoriaLibres(int cantidad,GBloque* disco, char* nombreParticion){
//Commons andan mal bitmap lee medio raro
    t_bitarray* bitmap = obtenerBitMap(nombreParticion, disco);

    t_list * bloquesLibres = list_create();

    int pos;

    //La correccion es por un error de las commons lee al reves cada byte
    //Osea lee los byte bien, pero para leer cada bit los lee de atras para
    //adelante. Por cada Byte lo tenes que adelantar 7 y ir restando en -1 la correccion
    int correccion = 7;
    for( pos = 0; pos < bitarray_get_max_bit(bitmap) ; pos++){

        if(0 ==  bitarray_test_bit(bitmap, pos + correccion)){
            int* bloqueLibre = malloc(sizeof(int));
            bitarray_set_bit(bitmap,pos + correccion);
            *bloqueLibre = pos;
            list_add(bloquesLibres, (void *)bloqueLibre);
        }
        if(list_size(bloquesLibres) == cantidad){
            break;
        }
        correccion-= 2;
        if((pos+1)%8 == 0){
            correccion = 7;
        }
    }

    return bloquesLibres;

}

GBloque* mapParticion (char* particion){

    int disco_size = obtenerTamanioArchivo(particion);

    int disco_file = open(particion, O_RDWR, 0);

    //Asigna archivos a memoria (Mapea el .bin a memoria ram)
    return mmap(NULL, disco_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,disco_file, 0);
}

void munmapParticion (GBloque* disco, char* nombreParticion){

    int disco_size = obtenerTamanioArchivo(nombreParticion);

    return munmap(disco,disco_size);
}

char* obtenerFechaActual(){
    time_t t;
    struct tm *tm;
    char* fechayhora = malloc(sizeof(char)*8);

    t=time(NULL);
    tm=localtime(&t);
    //No se porque 16 pero asi funciona --__(*.*)__--
    strftime(fechayhora, 16, "%d%m%Y", tm);

    return fechayhora;
}

int obtenerTamanioArchivo (char* file ){
    //Si devuelve null seguro esta en una carpeta arriba el archivo
    FILE* fd = fopen(file,"r");

    if(fd == NULL){
        return -1;
    }

    fseek(fd, 0L, SEEK_END);
    int32_t dfSize = ftell(fd);

    fclose(fd);
    return dfSize;
}

int obtenerNBloquesBitMap(double disco_size){
    //Formula dada por la consigna.
    double numeroBloquesNecesarios = (disco_size/BLOQUE_TAMANIO)/8/BLOQUE_TAMANIO;
    int parteEntera = numeroBloquesNecesarios / 1;
    double  parteDecimal = numeroBloquesNecesarios - (double)parteEntera;

    //Si la parte decimal es mayor a 0 le agrego un bloque mas
    if( parteDecimal > 0){
        parteEntera++;
    }
    return parteEntera;
}

int obtenerCantidadBytsBitmap(int disco_size){
    return  (disco_size/BLOQUE_TAMANIO)/8;
}

void inicializarBloqueDirectorio(GBloque* bloque){
    int* arrayDirecciones = (int*) bloque;

    for(int i = 0; i < (BLOQUE_TAMANIO/ sizeof(int));i++){
        arrayDirecciones[i] = -1;
    }
}

void crearDirectorioRaiz(GFile* nodo, GBloque* disco, char* nombreParticion) {

    //Inizializo el dir Raiz
    nodo->ptr_bloque_padre = -1;
    memcpy(nodo->nombre_archvio, "", strlen(""));
    nodo->estado = 2;
    nodo->size = 0;
    nodo->fecha_creacion = nodo->fecha_modificacion = obtenerFechaActual();

    //Busco un bloque libre para que guarde todos los puntero a los archivos
    t_list* bloqueLibre = buscarBloquesMemoriaLibres(1, disco, nombreParticion);
    int nro_nodoLibre = *(int*)list_get(bloqueLibre,0);
    inicializarBloqueDirectorio(disco + nro_nodoLibre);//Dir Bloque = inicio + nro_bloque
    nodo->GBloque[0] = nro_nodoLibre;

}

char* crearBitMap(int byts_bitmap){

    char* bitarray = malloc(byts_bitmap);
    int cantidad_bits = byts_bitmap * 8;


    t_bitarray* bitmap = bitarray_create_with_mode(bitarray, byts_bitmap, MSB_FIRST);

    //Inicializo todos los bits en 0
    for(int pos = 0; pos <= cantidad_bits; pos++){;
        bitarray_clean_bit(bitmap, pos);
    }
    //Seteo los bloques usados en el bitarray
    for(int pos = 0; pos < 1 + obtenerNBloquesBitMap(byts_bitmap * 8 *BLOQUE_TAMANIO) + (CANTIDAD_ARCHIVOS_MAX*sizeof(GFile)/BLOQUE_TAMANIO); pos++){
        bitarray_set_bit(bitmap, pos);
    }

    return bitmap->bitarray;
}


/*****************************
 **** Funciones formateo *****
 *****************************
 */

//Esta medio inestable usar las sac-tool
int formatear (char* nombre_particion, t_log* logger){

    int disco_size = obtenerTamanioArchivo(nombre_particion);

    if(disco_size == -1){
        log_error(logger, "Error al abrir el archivo");
        return -1;
    }

    int bitmap_count_bloques = obtenerNBloquesBitMap(disco_size);

    //Asigna archivos a memoria (Mapea el .bin a memoria ram)
    GBloque* disco = mapParticion (nombre_particion);

    escribirHeader(disco,bitmap_count_bloques);
    log_trace(logger, "Se termino de escribir el header en la direccion:%p",disco );

    escribirBitMap(disco + 1, bitmap_count_bloques, obtenerCantidadBytsBitmap(disco_size) );
    log_trace(logger, "Se termino de escribir el BitMap en la direccion:%p",disco + 1 );

    escribirNodeTabla(disco + 1 + bitmap_count_bloques);
    log_trace(logger, "Se termino de escribir la tabla de nodos en la direccion:%p",disco + 1 + bitmap_count_bloques );

    crearDirectorioRaiz(disco + 1 + bitmap_count_bloques, disco, nombre_particion);
    log_trace(logger, "Se escribio el directroio raiz en el nodo:%d", 0);

    munmap(disco,disco_size);
    return 0;
}

void escribirHeader(GBloque* puntero_disco, int bitmap_size){
    GHeader* newHeader = (GHeader *) puntero_disco;
    memcpy(newHeader->identificador, "SAC",IDENTIFICADOR_TAMANIO);
    newHeader->version = 1;
    newHeader->bitmap_inicio = 1;
    newHeader->bitMap_tamanio = bitmap_size;
}

void escribirBitMap(GBloque* disco, int  bitmap_bloques_count, int bitmap_size){

    char* bitmap = crearBitMap(bitmap_size);

    //Voy copiando el bitmap por partes en los distintos bloques
    for(int bloque = 0; bloque < bitmap_bloques_count ;bloque++){
        memcpy(disco[bloque].bytes, bitmap + (bloque * BLOQUE_TAMANIO), BLOQUE_TAMANIO);
    }

}

void escribirNodeTabla (GBloque* puntero_disco){

    GFile* nodo = (GFile *) puntero_disco;
    int tablaNodosPosition =nodo;

    //Inicializo toda la tabal de nodos con estado vacio
    for(int numero_archivo = 0; numero_archivo < CANTIDAD_ARCHIVOS_MAX; numero_archivo++){
        nodo[numero_archivo].estado = 0;
        nodo[numero_archivo].size = 0;
        (nodo[numero_archivo].nombre_archvio)[0]='\0';
    }
}


/*****************************
 **** Funciones Mostrar ******
 *****************************
 */

void mostrarParticion(char* nombre_particion){

    int disco_size = obtenerTamanioArchivo(nombre_particion);
    if(disco_size == -1){
        return;
    }

    int bitmap_count_bloques = obtenerNBloquesBitMap(disco_size);

    //Uso open porque necesito la direccion de memoria para memcopy
    int disco_file = open(nombre_particion, O_RDWR, 0);

    int bitmap_size = obtenerCantidadBytsBitmap(disco_size);

    GBloque* disco = mmap(NULL, disco_size, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,disco_file, 0);

    mostrarHeader(disco);

    mostrarBitMap(disco + 1,bitmap_size);

    mostrarTablaNodos(disco + 1 + obtenerNBloquesBitMap(disco_size));

    printf("\n------------------------ fin particion ------------------------");

    munmap(disco,disco_size);
}

void mostrarHeader(GBloque* disco ){
    GHeader* header = (GHeader *) disco;
    printf("Header \n\n");
    printf("Identificador: %s\nVersion: %d\nInicio BitMap: %d\nTamaño BitMap: %d\n", header->identificador, header->version, header->bitmap_inicio, header->bitMap_tamanio);
    printf("\n");
}

void mostrarBitMap(GBloque* disco, int bitmap_size){

    char* bitArray = malloc(bitmap_size);
    t_bitarray* bitmap = bitarray_create_with_mode(bitArray, bitmap_size, MSB_FIRST);
    memcpy(bitArray, disco, bitmap_size);
    printf("BitMap\n\n");
    int bits_usados = 0;
    int bits_libres = 0;
    int pos_bit;
    int contador_byts = 1;

    for(pos_bit = 0; pos_bit < bitmap_size*8; pos_bit++){
        int valor_bit = bitarray_test_bit(bitmap, pos_bit);
        if(valor_bit > 0){
            bits_usados++;
        }else{
            bits_libres++;
        }
        if((pos_bit % 8) == 0){
            printf("\n%d)",contador_byts);
            contador_byts++;
        }
        printf("%d",bitarray_test_bit(bitmap, pos_bit));
    }
    printf("\n \n");
    printf("Bloques: %d\t Bloques usados:%d\t Bloques libres: %d\n", pos_bit, bits_usados, bits_libres);
    printf("\n");

}

void mostrarTablaNodos(GBloque* disco){
        GFile* nodo = (GFile *) disco;
        printf("Tabal de nodos \n");
        for(int file = 0; file < CANTIDAD_ARCHIVOS_MAX; file ++){
            if(nodo[file].estado != 0){
                printf("\nNumero:%d\t Estado:%d\t Tamanio:%d\t Padre:%d\t Nombre:%s\t", file+1, nodo[file].estado, nodo[file].size, nodo[file].ptr_bloque_padre, nodo[file].nombre_archvio);
            }
        }
}

void mostrarNodo(GFile* nodo,GBloque* disco){
    printf("\nNombre: %s\n", nodo->nombre_archvio);
    printf("Tamanio: %d\n", nodo->size);
    printf("Estado: %d\n", nodo->estado);
    printf("Bloque padre: %d\n", nodo->ptr_bloque_padre);
    printf("Fecha creacion: %s\n", nodo->fecha_creacion);
    printf("Fecha modificacion: %s\n", nodo->fecha_modificacion);
    if(nodo->estado == 2){
        printf("Directorio:");
        int* array_archivos = (int*) (disco + nodo->GBloque[0]);
        for(int i = 0; i < (BLOQUE_TAMANIO/ sizeof(int));i++){
            if(*(array_archivos+i) != -1) {
                printf("\n\tArchivo %d: %d", i, *(array_archivos + i));
            }
        }
        printf("\n");
    }

}



//int main(int argc, char *argv[]){
//    t_log* logger = log_create("formateo.log", "SAC", 0, LOG_LEVEL_TRACE);
//
//
//    formatear("../disco.bin",logger);
//
//
//
//    mostrarParticion("../disco.bin");
//
//
//    log_destroy(logger);
//    return 0;
//}