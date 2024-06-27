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
#include<commons/temporal.h>
#include<commons/collections/dictionary.h>

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
    SALIR,
    RESIZE,
    OUT_OF_MEMORY
};

enum comunicacion_io_tipos{
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
};

enum comunicacion_io_ins{
    IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,    
    IO_FS_TRUNCATE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_READ,
    IO_FS_WRITE
};

struct recursosYPCB{
    PCB* pcb;
    t_dictionary* recursos;
    char* bloqueando;
};

typedef struct recursosYPCB rec;
struct interfaz{
    char* nombre;
    int tipo;
    bool libre;
    int conexion;
    t_list* cola;
    sem_t trigger;
};

struct cola_de_operaciones{
    PCB* pcb;
    int operacion;
    void* parametro;
};

typedef struct interfaz interfaz;
//typedef struct cola_de_operaciones cola_de_operaciones;

t_list *ready;
t_list *new;
t_list *blocked;
t_list *readyQuantum;

t_list *recursos_por_proceso;
sem_t mutex_listas;


PCB *execute;


void eliminar_interfaz(interfaz* i);
void desbloquearProceso(PCB* proceso);


void planFIFO(void);
void planRR(void);
void interrupcionesRR(PCB proceso);
t_list* enviar_al_CPU(PCB* a_ejecutar);
void iniciar_planificaciones();
void atender_syscall(t_list* lista);
void operaciones_de_interfaz(interfaz* i);
interfaz* encontrarInterfaz(char* nombre,int tipo);
void exit_execute(char * razon);
void bloquear_execute(char* nombre);
void memoria_liberar_proceso(int pid);
void inicializar_recursos(void);
void inicializar_recursos_del_proceso(PCB* pcb);
bool elProcesoTieneUnrecurso(rec *recu ,char* nombre);
void sacarPrimerPCB();
bool encontrar_recursos_del_execute(void *r);
void liberar_procesos_bloqueados_por_recursos(char* nombre);
void loggear_lista(t_list *lista);
void controlar_interfaces();



#endif