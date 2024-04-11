#ifndef _MEMORIA_H
#define _MEMORIA_H

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <commons/config.h>
#include <pthread.h>


pthread_t cpu;
pthread_t io;
pthread_t kernel;

void comunicacion_cpu(int conexion);
void comunicacion_kernel(int conexion);
void comunicacion_io(int conexion);

#endif