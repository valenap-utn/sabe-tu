#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include "utils.h"
#include "pthread.h"
#include <commons/string.h>
#include <unistd.h>
#include <sys/time.h>

enum comunicacion_kernel_tipos{
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
};

enum comunicacion_io{
    ESCRIBIR,
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


void inter(char *nombre);
int tipoInter(char* s);


#endif 