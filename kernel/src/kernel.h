#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <commons/config.h>
#include <commons/collections/list.h>
#include <pcb.h>
#include<pthread.h>
#include<semaphore.h>

enum conucicacion_cpu
{
    ESPERAR,
    SENIAL,
    DORMIR,
    IO_LEER,
    IO_ESCRIBIR,
    CREAR,
    BORRAR,
    TRUNCAR,
    FS_ESCRIBIR,
    FS_LEER,
    SALIR
};

t_list *ready;
t_list *new;
t_list *blocked;

PCB *execute;
void planFIFO(void);
void planRR(void);
void interrupcionesRR(PCB proceso);
t_list* enviar_al_CPU(PCB* a_ejecutar);
void iniciar_planificaciones();
void atender_syscall(t_list* lista);


#endif