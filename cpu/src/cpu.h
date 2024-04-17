#ifndef _CPU_H
#define _CPU_H


#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include<pthread.h>

int conexion_memoria;

uint32_t PC;
int PID;
uint8_t AX;
uint8_t BX;
uint8_t CX;
uint8_t DX;
uint32_t EAX;
uint32_t EBX;
uint32_t ECX;
uint32_t EDX;
uint32_t SI;
uint32_t DI;

int interrupcion = 1;



pthread_t interrupciones;
pthread_t dispatch;

void interrupt();
void instrucciones();
void actualizar_registros(int conexion);
void devolver_contexto(int conexion);

#endif