#include"kernel.h"

    int conexion_memoria;
    int conexion_cpu_dispatch;
    int conexion_cpu_interrupt;
    t_list* interfaces;


int main(int argc, char* argv[]) {
    
    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/kernel/kernel.config");
    logger = log_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/kernel/Kernel.log","KERNEL",1,LOG_LEVEL_INFO);

    ready = list_create();
    new = list_create();
    blocked =list_create();
    interfaces = list_create();

    iniciar_planificaciones();



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
        sem_wait(&mutex_listas);
        execute = list_remove(ready,0);
        sem_post(&mutex_listas);

        t_list* lista = enviar_al_CPU(execute);
        atender_syscall(lista);
    }
}

void controlar_interfaces()
{
    int server_fd = iniciar_servidor();
    while(1)
    {
        int conexion_I_O = esperar_cliente(server_fd);
        {
            int i;
            recv(conexion_I_O,&i,sizeof(int),MSG_WAITALL);
            send(conexion_I_O,&i,sizeof(int),0);
        }
        interfaz *i = malloc(sizeof(interfaz));
        i->conexion = conexion_I_O;
        i->libre = true;
        i->cola = list_create();
        sem_init(&i->trigger,0,0);
        recv(conexion_I_O,&i->tipo,sizeof(int),MSG_WAITALL);
        
        int tamanio;
        recv(conexion_I_O,&tamanio,sizeof(int),MSG_WAITALL);
        char nombre[tamanio];
        recv(conexion_I_O,nombre,tamanio,MSG_WAITALL);
        char* nombrePosta = string_new();
        string_append(&nombrePosta,nombre);

        i->nombre = nombrePosta;

        list_add(interfaces,i);

        pthread_t hilo;

        pthread_create(&hilo,NULL,(void*)operaciones_de_interfaz,i);
    }
}

void operaciones_de_interfaz(interfaz* i)
{
    while(1)
    {
        sem_wait(&i->trigger);

        int comunicacion;
        struct cola_de_operaciones *op = list_remove(i->cola,0);
        switch (op->operacion)
        {
        case DORMIR:
            comunicacion = IO_GEN_SLEEP;
            send(i->conexion,&comunicacion,sizeof(int),0);

            send(i->conexion,op->parametro,sizeof(int),0);


            recv(i->conexion,NULL,sizeof(int),MSG_WAITALL);
            i->libre = true;
            desbloquearProceso(op->pcb);
            break;
        case -1:
            eliminar_interfaz(i);
            break;
        default:

            break;
        }
    }
}

void eliminar_interfaz(interfaz *i)
{
    list_destroy(i->cola);
    free(i->nombre);
    sem_destroy(&i->trigger);
    free(i);
}

void desbloquearProceso(struct PCB* proceso)
{
	sem_wait(&mutex_listas);
	list_remove_element(blocked,proceso);
	list_add(ready,proceso);
	sem_post(&mutex_listas);
	log_info(logger,"PID: <%d> - Estado Anterior: <BLOCKED> - Estado Actual: <READY>",proceso->pid);
	// sem_post(&hay_cosas_en_ready);
}

void planRR(void)
{
    while(1)
    {
        sem_wait(&mutex_listas);
        execute = list_remove(ready,0);
        sem_post(&mutex_listas);
        pthread_t interrupciones;
        pthread_create(&interrupciones,NULL,(void*)interrupcionesRR,execute);


        t_list* lista = enviar_al_CPU(execute);
        if(lista == NULL)
        {
            sem_wait(&mutex_listas);
            list_add(ready,execute);
            sem_post(&mutex_listas);
        }
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
	usleep(proceso.quantum*1000);
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
            exit_execute();
        break;
        case DORMIR:
            char *nombre_int = list_remove(lista,1);
            interfaz *i = encontrarInterfaz(nombre_int,GENERICA);
            if(i != NULL)
            {
                struct cola_de_operaciones* op = malloc(sizeof(struct cola_de_operaciones));
                op->pcb = execute;
                op->parametro = list_remove(lista,1);
                op->operacion = DORMIR;

                bloquear_execute(nombre_int);

                sem_post(&i->trigger);
            }
        break;
    }
}

void bloquear_execute(char* nombre)
{
	sem_wait(&mutex_listas);
	list_add(blocked,execute);
	sem_post(&mutex_listas);
	log_info(logger,"PID: <%d> - Estado Anterior: <EXECUTE> - Estado Actual: <BLOCKED>",execute->pid);
	log_info(logger,"PID: <%d> - Bloqueado por: <%s>",execute->pid,nombre);
	// sem_post(&bloqueos);
}

interfaz* encontrarInterfaz(char* nombre,int tipo)
{   
    bool comparar(void* i)
    {
        return string_equals_ignore_case(((interfaz*)i)->nombre, nombre);
    };

    interfaz *i = list_find(interfaces,comparar);
    if(i->tipo != tipo && i == NULL)
    {
        exit_execute();
        return NULL;
    }

    return i;
}

void exit_execute()
{
    	// desbloquear_todo(execute);
		// sacar_de_laMatriz(execute->PID);
        memoria_liberar_proceso(execute->pid);
		free(execute);
		// sem_post(&espacio_libre_en_colas);
}

void memoria_liberar_proceso(int pid)
{
	int i = FINALIZACION;

    send(conexion_memoria,&i,sizeof(int),0);
    send(conexion_memoria,&pid,sizeof(int),0);
}