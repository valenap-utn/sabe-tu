#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_

#include <stdint.h>
#include "utils.h"
#include<commons/string.h>


enum conucicacion_kernel
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

extern uint32_t PC;
extern int PID;
extern uint8_t AX;
extern uint8_t BX;
extern uint8_t CX;
extern uint8_t DX;
extern uint32_t EAX;
extern uint32_t EBX;
extern uint32_t ECX;
extern uint32_t EDX;
extern uint32_t SI;
extern uint32_t DI;

extern bool sysCall;

extern t_paquete* paquete;

#endif