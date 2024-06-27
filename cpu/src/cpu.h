#ifndef _CPU_H
#define _CPU_H


#include <stdlib.h>
#include <stdio.h>
#include<pthread.h>
#include "instrucciones.h"



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





int interrupcion = 1;


pthread_t interrupciones;
pthread_t dispatch;

void interrupt();
void instrucciones();
void actualizar_registros(int conexion);
void devolver_contexto(int conexion);
char* fetch(int conexion);
t_list *decode(char* instruccion);
void list_add_strAReg1_strtoul2(t_list* operandos, char* traduccion1, char* traduccion2);
void list_add_strAReg_1y2(t_list* operandos, char* traduccion1, char* traduccion2);
void list_add_trad1_strAReg2y3(t_list* operandos, char* traduccion1, char* traduccion2, char* traduccion3);
void list_add_trad1y2(t_list* operandos, char* traduccion1, char* traduccion2);
void list_add_strAReg34y5(t_list* operandos, char* traduccion3, char* traduccion4, char* traduccion5);
void *stringAregistro(char* registro);
void execute(t_list *instruccion);

#endif