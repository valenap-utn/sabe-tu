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


    iniciar_planificaciones();
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
        char** comando = iniciarBash();
        switch (procesar_comando(comando[0]))
        {
        case INICIAR_PROCESO:
            PCB* pcb = iniciar_pcb(config_get_int_value(config,"QUANTUM"));
            paquete_a_memoria(comando[1],conexion_memoria,pcb->pid);
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
        matar_comando(comando);
    }

    return 0;
}

void planFIFO()
{
    while(1)
    {
        execute = list_remove(ready,0);

        t_list* lista = enviar_al_CPU(execute);
        atender_syscall(lista);
    }
}

void planRR(void)
{
    while(1)
    {
        execute = list_remove(ready,0);
        pthread_t interrupciones;
        pthread_create(&interrupciones,NULL,(void*)interrupcionesRR,execute);


        t_list* lista = enviar_al_CPU(execute);
        atender_syscall(lista);
    }
}

t_list* enviar_al_CPU(PCB* a_ejecutar)
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

    bool paq;
    recv(conexion_cpu_dispatch,&(paq),sizeof(bool),MSG_WAITALL);
    if(paq)return recibir_paquete(conexion_cpu_dispatch);
    return NULL;
}

void iniciar_planificaciones(void)
{
    pthread_t cortoPlazo;

    char *planificacion = string_new();
    string_append(&planificacion,config_get_string_value(config,"ALGORITMO_PLANIFICACION"));
    if(string_equals_ignore_case(planificacion,"fifo"))pthread_create(&cortoPlazo,NULL,(void*)planFIFO,NULL);
    if(string_equals_ignore_case(planificacion,"rr")) pthread_create(&cortoPlazo,NULL,(void*)planRR,NULL);
}

void interrupcionesRR(PCB proceso)
{

	// pthread_mutex_lock(&para_frenar);
	// pthread_mutex_unlock(&para_frenar);
    int i = proceso.pid;
	usleep(proceso.quantum);
    if(execute!=NULL)
	{	
		if(i == execute->pid)
		{
            i = 0;
			send(conexion_cpu_interrupt,&i,sizeof(int),0);
			log_info(logger,"PID: <%d> - Desalojado por fin de Quantum",execute->pid);
		}
	}
}


void atender_syscall(t_list* lista)
{
    switch((int)list_get(lista,0))
    {
        case SALIR:

        break;
        case DORMIR:

        break;
    }
}