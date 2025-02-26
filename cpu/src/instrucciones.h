#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_

#include <stdint.h>
#include <commons/string.h>
#include "TLB.h"
#include <math.h>


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
    SALIR,
    OUT_OF_MEMORY
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

extern int tam_pagina;

extern bool sysCall;

extern t_paquete* paquete;

void set(void* registro, uint32_t valor);
void mov_in(void* registro_datos,void* registro_direccion);
void mov_out(void* registro_datos,void* registro_direccion);
void sum(void* destino,void* origen);
void sub(void* destino,void* origen);
void jnz(void* registro,uint32_t ins);
void resize(int tamanio);
void copy_string(void* tamanio);
void wait(char* recurso);
void s1gnal(char* recurso);
void io_gen_sleep(char* interfaz,int unidad_trabajo);
void io_stdin_read(char* interfaz,void* registro_direccion,void* registro_tamanio);
void io_stdout_write(char* interfaz,void* registro_direccion,void* registro_tamanio);
void io_fs_create(char* interfaz,char* nombre_archivo);
void io_fs_delete(char* interfaz,char* nombre_archivo);
void io_fs_truncate(char* interfaz,char* nombre_archivo,void* registro_tamanio);
void io_fs_write(char* interfaz,char* nombre_archivo,void* registro_direccion,void* registro_tamanio,void* registro_puntero_archivo);
void io_fs_read(char* interfaz,char* nombre_archivo,void* registro_direccion,void* registro_tamanio,void* registro_puntero_archivo);
void salir();
int tamanio(void* registro);

uint32_t traducir_direccion(int direccion_logica);


#endif