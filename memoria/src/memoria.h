#ifndef _MEMORIA_H
#define _MEMORIA_H

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <commons/config.h>
#include <pthread.h>
#include<commons/collections/list.h>
#include<commons/string.h>

enum comunicacion_con_kernel{
    CREACION,
    FINALIZACION,
    AJUSTAR
};

enum comunicacion_con_cpu
{
    INSTRUCCION,
    MARCO,
    LECTURA,
    ESCRITURA
};

struct proceso{
    int pid;
    char* nombre;
    t_list* instrucciones;
    int tamanio;
};

typedef struct proceso proceso;

t_list* procesos;

pthread_t cpu;
pthread_t io;
pthread_t kernel;

void comunicacion_cpu(int conexion);
void comunicacion_kernel(int conexion);
void comunicacion_io(int conexion);
struct proceso *guardar_proceso(int conexion);
bool cmpProcesoId(void *p);

#endif