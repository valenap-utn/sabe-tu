#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include "utils.h"
#include "pthread.h"
#include <commons/string.h>
#include <unistd.h>
#include <sys/time.h>
#include<readline/readline.h>
#include <stdlib.h>
#include <stdio.h>
#include<commons/bitarray.h>
#include<math.h>

enum comunicacion_kernel_tipos{
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
};

enum comunicacion_io{
    ESCRIBIR = 1,
    LEER
};

enum comunicacion_kernel_ins{
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,    
    IO_FS_TRUNCATE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_READ,
    IO_FS_WRITE
};
typedef struct interfaz interfaz;

struct archivo{
    char* nombre;
    int bloque_inicial;
    int tamanio;
};

typedef struct archivo archivo;

void inter(int conexion_kernel);
void interfs();
int tipoInter(char* s);

void inicializar_bloques(char* path_bloques);
void inicializar_bitmap(char* path_bitmap);
int asignar_espacio(archivo *archivo,int tamanio,int pid, int bloques_libres);
int cargar_bitmap(char* path);
int bloque_inicial();

void liberar_espacio(int bloque_inicial, int tamanio_actual, int tamanio_final);
void vaciar_bloque(int bloque);
void cargar_bloques(char* path);

void actualizar_archivo_bloques();
void actualizar_archivo_bitmap();

void compactacion(archivo* archivo1,int pid);
void compactar_bitmap(int bloques_ocupados);

bool bloque_inicial_archivo(void* archivo1,void* archivo2);
bool comparar_archivo(void* archivo1);
void vaciar_bloque(int bloque);

void actualizar_bitmap(int inicial_limpiar, int tamanio_limpiar, int inicial_settear, int tamanio_settear);
void actualizar_metadata(archivo* archivo);

#endif 