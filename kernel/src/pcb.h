#ifndef PCB_H_
#define PCB_H_

#include <commons/string.h>
#include <stdint.h>

enum posibles_comandos{
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    MULTIPROGRAMACION,
    PROCESO_ESTADO,
    EJECUTAR_SCRIPT,
};

struct PCB
{
    int pid;
    int quantum;
    uint32_t PC;
    uint8_t AX;
    uint8_t  BX;
    uint8_t  CX;
    uint8_t  DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
};

typedef struct PCB PCB;

int pids = 1;

char** iniciarBash();
int procesar_comando(char* comando);
PCB* iniciar_pcb();

#endif