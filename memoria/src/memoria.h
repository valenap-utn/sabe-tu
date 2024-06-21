#ifndef _MEMORIA_H
#define _MEMORIA_H

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <commons/config.h>
#include <pthread.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<math.h>

enum comunicacion_con_kernel{
    CREACION,
    FINALIZACION
   // AJUSTAR
};

enum comunicacion_con_cpu
{
    INSTRUCCION,
    MARCO,
    LECTURA,
    ESCRITURA,
    AJUSTE
};

enum comunicacion_io{
    ESCRIBIR,
    LEER
};
struct proceso{
    int pid;
    char* nombre;
    t_list* instrucciones;
    int tamanio;
    t_list* paginas;
};

typedef struct proceso proceso;

t_list* procesos;

pthread_mutex_t mutex_pid;
pthread_t cpu;
pthread_t io;
pthread_t kernel;

void comunicacion_cpu(int conexion);
void comunicacion_kernel(int conexion);
void comunicacion_io(void);

struct proceso *guardar_proceso(int conexion);
bool cmpProcesoId(void *p);
void atender_io(int conexion);

void iniciar_paginasOcupadas();

uint32_t bytes_to_uint32(const unsigned char bytes[4]);

bool modificar_paginas_proceso(proceso* p,int new_tam);

void* leer_peticion(int pid, int direccion, int tamanio);
void recibir_escritura(int conexion,int direccion,int tamanio,int pid,char* mensaje);
int proximo_marco(int Pid, int actual);
int min(int a, int b);
char* uint32_to_bytes(uint32_t valor);
uint32_t bytes_to_uint32(const unsigned char bytes[4]);

#endif