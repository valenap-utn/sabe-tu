#include "pcb.h"
#include<sys/socket.h>
int pids = 1;

char** iniciarBash(){
    char *comando = string_new();
    string_append(&comando,readline(">"));
    string_trim(&comando);
    char ** partes_del_comando = string_split(comando," ");
    return partes_del_comando;
}

int procesar_comando(char* comando)
{
    if(string_equals_ignore_case(comando,"INICIAR_PROCESO")){return INICIAR_PROCESO;}
	if(string_equals_ignore_case(comando,"FINALIZAR_PROCESO")){return FINALIZAR_PROCESO;}
    if(string_equals_ignore_case(comando, "DETENER_PLANIFICACION")){return DETENER_PLANIFICACION;}
    if(string_equals_ignore_case(comando, "INICIAR_PLANIFICACION")){return INICIAR_PLANIFICACION;}
    if(string_equals_ignore_case(comando, "MULTIPROGRAMACION")){return MULTIPROGRAMACION;}
    if(string_equals_ignore_case(comando, "PROCESO_ESTADO"))return PROCESO_ESTADO;
    if(string_equals_ignore_case(comando, "EJECUTAR_SCRIPT"))return EJECUTAR_SCRIPT;
    return -1;
}


PCB *iniciar_pcb(int quantum)
{
    PCB* pcb = malloc(sizeof(PCB));
    pcb->PC = pids++;
    pcb->PC = 0;
    pcb->AX = 0;
    pcb->BX = 0;
    pcb->CX = 0;
    pcb->DX = 0;
    pcb->EAX = 0;
    pcb->EBX = 0;
    pcb->ECX = 0;
    pcb->EDX = 0;
    pcb->SI = 0;
    pcb->DI = 0;
    pcb->quantum = quantum;
    return pcb;
}

void matar_comando(char** comando)
{
    string_array_destroy(comando);
}

void paquete_a_memoria(char* comando,int conexion,int pid)
{
    int i = CREACION;
    send(conexion,&i,sizeof(int),0);
    i = string_array_size(&comando);
    send(conexion,&i,sizeof(int),0);
    send(conexion,comando,i,0);
    send(conexion,&pid,sizeof(int),0);
}