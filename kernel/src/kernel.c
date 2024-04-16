#include"kernel.h"

    int conexion_memoria;
    int conexion_I_O;
    int conexion_cpu_dispatch;
    int conexion_cpu_interrupt;

int main(int argc, char* argv[]) {
    
    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/kernel/kernel.config");
    logger = log_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/kernel/Kernel.log","KERNEL",1,LOG_LEVEL_INFO);

    ready = list_create();
    new = list_create();
    blocked =list_create();

    pthread_t cortoPlazo;

    pthread_create(&cortoPlazo,NULL,planFIFO,NULL);
    int server_fd = iniciar_servidor();
    log_info(logger,"hola soy el kernel y funciono");
	conexion_I_O = esperar_cliente(server_fd);
    {
        int i;
        recv(conexion_I_O,&i,sizeof(int),MSG_WAITALL);
        send(conexion_I_O,&i,sizeof(int),0);
    }


    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA","memoria");
    conexion_cpu_dispatch = conectar("PUERTO_CPU_DISPATCH","IP_CPU","cpu dispatch");
    conexion_cpu_interrupt = conectar("PUERTO_CPU_INTERRUPT","IP_CPU","cpu interrupt");
    log_info(logger, "Entrada y salida conectada");

    while(1)
    {
        char** comando = inicia_bash();
        switch (procesar_comando(comando[0]))
        {
        case INICIAR_PROCESO:
            PCB* pcb = iniciar_pcb();

            // send(conexion_memoria,comando[1],0,0);
            list_add(new,pcb);
        break;
        case FINALIZAR_PROCESO:

        break;
        case INICIAR_PLANIFICACION:

        break;
        case DETENER_PLANIFICACION:

        break;
        case MULTIPROGRAMACION:

        break;
        case PROCESO_ESTADO:
        

        break;
        case EJECUTAR_SCRIPT:

        break;
        default:
            log_error(logger,"consola mal utilizada");
        break;
        }
    }

    return 0;
}

void planFIFO()
{

    PCB* a_ejecutar = list_remove(ready,0);

    enviar_al_CPU(a_ejecutar);
}

void enviar_al_CPU(PCB* a_ejecutar)
{
    send(conexion_cpu_dispatch,&(a_ejecutar->pid),sizeof(int),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->PC),sizeof(uint32_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->AX),sizeof(uint8_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->BX),sizeof(uint8_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->CX),sizeof(uint8_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->DX),sizeof(uint8_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->EAX),sizeof(uint32_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->EBX),sizeof(uint32_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->ECX),sizeof(uint32_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->EDX),sizeof(uint32_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->SI),sizeof(uint32_t),0);
    send(conexion_cpu_dispatch,&(a_ejecutar->DI),sizeof(uint32_t),0);

    recv(conexion_cpu_dispatch,&(a_ejecutar->PC),sizeof(uint32_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->AX),sizeof(uint8_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->BX),sizeof(uint8_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->CX),sizeof(uint8_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->DX),sizeof(uint8_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->EAX),sizeof(uint32_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->EBX),sizeof(uint32_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->ECX),sizeof(uint32_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->EDX),sizeof(uint32_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->SI),sizeof(uint32_t),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&(a_ejecutar->DI),sizeof(uint32_t),MSG_WAITALL);
}