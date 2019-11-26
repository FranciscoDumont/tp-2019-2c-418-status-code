#include "tools.h"


/*****************************
 **** Funciones auxiliares****
 ****************************/

int obtenerBitMapSize(char* particion){
    int disco_size = obtenerTamanioArchivo(particion);
    return obtenerCantidadBytsBitmap(disco_size);
}

t_bitarray* obtenerBitMap(char* particion, GBloque* disco){

    int tamanioBitmap = obtenerBitMapSize(particion);

    return bitarray_create((char *)(disco + 1), tamanioBitmap);
}

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

    bitarray_destroy(bitmap);
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
    char* fechayhora = malloc(16);

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
    memcpy(nodo->nombre_archivo, "", strlen(""));
    nodo->estado = 2;
    nodo->size = 0;
    char* fechahoy = obtenerFechaActual();
    nodo->fecha_creacion = nodo->fecha_modificacion = atoi(fechahoy);

    //Busco un bloque libre para que guarde todos los puntero a los archivos
    t_list* bloqueLibre = buscarBloquesMemoriaLibres(1, disco, nombreParticion);
    int nro_nodoLibre = *(int*)list_get(bloqueLibre,0);
    inicializarBloqueDirectorio(disco + nro_nodoLibre);//Dir Bloque = inicio + nro_bloque
    nodo->GBloque[0] = nro_nodoLibre;

    free(list_get(bloqueLibre,0));
    free(fechahoy);
    list_destroy(bloqueLibre);

}

t_bitarray* crearBitMap(int byts_bitmap){

    char* bitarray = malloc(byts_bitmap);
    int cantidad_bits = byts_bitmap * 8;


    t_bitarray* bitmap = bitarray_create_with_mode(bitarray, byts_bitmap, MSB_FIRST);

    //Inicializo todos los bits en 0
    for(int pos = 0; pos < cantidad_bits; pos++){;
        bitarray_clean_bit(bitmap, pos);
    }
    //Seteo los bloques usados en el bitarray
    for(int pos = 0; pos < 1 + obtenerNBloquesBitMap(byts_bitmap * 8 *BLOQUE_TAMANIO) + (CANTIDAD_ARCHIVOS_MAX*sizeof(GFile)/BLOQUE_TAMANIO); pos++){
        bitarray_set_bit(bitmap, pos);
    }

    return bitmap;
}

//Busca un path en el disco
int buscarPath(t_list* pathDividido){

    //Inicializo el nodo resultado en error
    int nodoResultado = -1;

    //Veo si el path esta vacio
    if(!list_is_empty(pathDividido)) {
        //Si no lo esta  quiere decir que no es la raiz

        //Obtengo el nombre del archivo que es el ultimo en tokenizar
        char*nombreArchivo = (char*) list_get(pathDividido, list_size(pathDividido)-1);

        t_list* nodosCandidatos = hallar_posibles_nodos(nombreArchivo);

        //Mapeop el disco a memoria
        GBloque* disco = mapParticion("../tools/disco.bin");
        GFile* tablaNodos = obtenerTablaNodos(disco);


        //Me fijo de los candidatos cual cumple con el path
        for (int i = 0; i < list_size(nodosCandidatos); i++) {
            //Traigo el nodo candidato a memoria
            GFile *nodoCandidato = tablaNodos + (int) list_get(nodosCandidatos, i);
            GFile* nodoPadre = tablaNodos + nodoCandidato->ptr_bloque_padre;
            bool esNodo = false;

            //Reviso si el nodo candidato y el path tienen que estar en la raiz solamente
            if( nodoCandidato->ptr_bloque_padre == 0 && list_size(pathDividido) == 1 ){
                esNodo = true;
            }else {

                //Voy iterando los padres y viendo si coinciden con el path
                for (int j = 1; j < list_size(pathDividido); j++) {
                    char *nombreArchivo = (char *) list_get(pathDividido, list_size(pathDividido) - 1 - j);
                    if (strcmp(nodoPadre->nombre_archivo, nombreArchivo, strlen(nombreArchivo)) != 0) {
                        esNodo = false;
                        break;
                    }
                    esNodo = true;
                    nodoPadre = tablaNodos + nodoPadre->ptr_bloque_padre;
                }
            }
            //Si cumplio el nodo es el nodo resultado
            if(esNodo){
                nodoResultado = (int) list_get(nodosCandidatos, i);
                break;
            }
        }

        //Librero los recursos y demapeo la particion
        list_destroy(nodosCandidatos);
        munmapParticion (disco, "../tools/disco.bin");
    }else{
        nodoResultado = 0;
    }

    return  nodoResultado;
}

//Sirve para devolver los posibles nodos que contengan un nombre
t_list* hallar_posibles_nodos(char* nombreNodo){

    t_list* nodosCandidatos = list_create();
    GBloque* disco = mapParticion("../tools/disco.bin");//Seguro va a tener que ser una variable global

    GFile* nodo = obtenerTablaNodos(disco);
    for(int nro_nodo = 0 ; nro_nodo < 1024; nro_nodo++){
        //Si comparo los nombres y es verdadero los agrego a la lista de nodos candidatos
        if(strcmp((nodo + nro_nodo)->nombre_archivo,nombreNodo) == 0){
            list_add(nodosCandidatos, nro_nodo);
        }
    }

    munmapParticion(disco,"../tools/disco.bin");

    return nodosCandidatos;
}

//Te divide el path separado con '/' en una lista
t_list* dividirPath(char* path){
    char* token = strtok(path, "/");
    t_list* pathDividido = list_create();

    while(token != NULL){
        list_add(pathDividido, token);
        token = strtok(NULL, "/");
    }
    return pathDividido;
}

int obtenerNodoLibre (GFile* comienzoTabla){
    for(int i = 0; i  < CANTIDAD_ARCHIVOS_MAX; i++ ) {
        if ((comienzoTabla + i)->estado == 0) {
            return i;

        }
    }
    return -1;
}

GFile* obtenerTablaNodos(GBloque* comienzoParticion){
    GHeader* header = (GHeader*)comienzoParticion;

    //Y saco del header el tamanio del bitmap
    //Corro entonces el puntero 1 bloque mas los bloques que ocupa el bitMap
    return (GFile *) header + 1 + header->bitMap_tamanio;
}
int calcularCorreccion(int nro_bloque){
    int numero_correcto = nro_bloque;
    switch(nro_bloque % 8){
        case 0:
            numero_correcto += 7;
            break;
        case 1:
            numero_correcto += 5;
            break;
        case 2:
            numero_correcto += 3;
            break;
        case 3:
            numero_correcto += 1;
            break;
        case 4:
            numero_correcto -= 1;
            break;
        case 5:
            numero_correcto -= 3;
            break;
        case 6:
            numero_correcto -= 5;
            break;
        case 7:
            numero_correcto -= 7;
            break;
    }
    return numero_correcto;
}

void liberarBloqueMemoria(int bloque, GBloque* disco, char* nombreParticion){
    t_bitarray* bitmap = obtenerBitMap(nombreParticion, disco);

    bitarray_clean_bit(bitmap, bloque);

    bitarray_destroy(bitmap);

}

/*****************************
 **** Funciones formateo *****
 ****************************/

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

    t_bitarray* bitmap = crearBitMap(bitmap_size);

    //Voy copiando el bitmap por partes en los distintos bloques
    for(int bloque = 0; bloque < bitmap_bloques_count ;bloque++){
        memcpy(disco[bloque].bytes, bitmap->bitarray + (bloque * BLOQUE_TAMANIO), BLOQUE_TAMANIO);
    }

    free(bitmap->bitarray);
    bitarray_destroy(bitmap);

}

void escribirNodeTabla (GBloque* puntero_disco){

    GFile* nodo = (GFile *) puntero_disco;
    int tablaNodosPosition =nodo;

    //Inicializo toda la tabal de nodos con estado vacio
    for(int numero_archivo = 0; numero_archivo < CANTIDAD_ARCHIVOS_MAX; numero_archivo++){
        nodo[numero_archivo].estado = 0;
        nodo[numero_archivo].size = 0;
        (nodo[numero_archivo].nombre_archivo)[0]='\0';
        for(int j=0; j < 1000; j++){
            nodo[numero_archivo].GBloque[j] = -1;
        }
    }
}


/*****************************
 **** Funciones Mostrar ******
 ****************************/

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

    bitarray_destroy(bitmap);
    free(bitArray);

}

void mostrarTablaNodos(GBloque* disco){
        GFile* nodo = (GFile *) disco;
        printf("Tabal de nodos \n");
        for(int file = 0; file < CANTIDAD_ARCHIVOS_MAX; file ++){
            if(nodo[file].estado != 0){
                printf("\nNumero:%d\t Estado:%d\t Tamanio:%d\t Padre:%d\t Nombre:%s\t", file, nodo[file].estado, nodo[file].size, nodo[file].ptr_bloque_padre, nodo[file].nombre_archivo);
            }
        }
}

//void mostrarNodoDirectorio(GFile* nodo,GBloque* disco){
//    printf("\nNombre: %s\n", nodo->nombre_archvio);
//    printf("Tamanio: %d\n", nodo->size);
//    printf("Estado: %d\n", nodo->estado);
//    printf("Bloque padre: %d\n", nodo->ptr_bloque_padre);
//    printf("Fecha creacion: %d\n", nodo->fecha_creacion);
//    printf("Fecha modificacion: %d\n", nodo->fecha_modificacion);
//    printf("Bloque que usa: %d\n", nodo->GBloque[0]);
//    if(nodo->estado == 2){
//        printf("Directorio:");
//        int* array_archivos = (int*) (disco + nodo->GBloque[0]);
//        for(int i = 0; i < (BLOQUE_TAMANIO/ sizeof(int));i++){
//            if(*(array_archivos+i) != -1) {
//                printf("\n\tArchivo %d: %d", i, *(array_archivos + i));
//            }
//        }
//        printf("\n");
//    }
//
//}


void mostrarNodo(GFile* nodo,GBloque* disco){
    printf("\nNombre: %s\n", nodo->nombre_archivo);
    printf("Tamanio: %d\n", nodo->size);
    printf("Estado: %d\n", nodo->estado);
    printf("Bloque padre: %d\n", nodo->ptr_bloque_padre);
    printf("Fecha creacion: %d\n", nodo->fecha_creacion);
    printf("Fecha modificacion: %d\n", nodo->fecha_modificacion);
    if(nodo->estado == 2){
        printf("Bloque que usa: %d\n", nodo->GBloque[0]);
        printf("Directorio:");
        if(nodo->GBloque[0] != -1){
            int* array_archivos = (int*) (disco + nodo->GBloque[0]);
            for(int i = 0; i < (BLOQUE_TAMANIO/ sizeof(int));i++){
                if(*(array_archivos+i) != -1) {
                    printf("\n\tArchivo %d: %d", i, *(array_archivos + i));
                }
            }
        }
        printf("\n");
    }else{
        printf("Bloques que usa:\n");
        for(int i = 0; i < 1000;i++){
            if(nodo->GBloque[i] !=-1){
                printf("\tBloque %d: %d\n", i, nodo->GBloque[i]);
            }
        }
    }

}
