#ifndef _CPU_H
#define _CPU_H


#include <stdlib.h>
#include <stdio.h>
#include<pthread.h>

#include "instrucciones.h"

enum comunicacion_con_memoria
{
    INSTRUCCION,
    MARCO,
    LECTURA,
    ESCRITURA
};

enum instrucciones
{
    SET,
    MOV_IN,
    MOV_OUT,
    SUM,
    SUB,
    JNZ,
    RESIZE,
    COPY_STRING,
    WAIT,
    SIGNAL,
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_READ,
    IO_FS_WRITE,
    IO_FS_TRUNCATE,
    EXIT
};

int conexion_memoria;


int interrupcion = 1;


pthread_t interrupciones;
pthread_t dispatch;

void interrupt();
void instrucciones();
void actualizar_registros(int conexion);
void devolver_contexto(int conexion);
char* fetch(int conexion);
t_list *decode(char* instruccion);
void *stringAregistro(char* registro);

#endif