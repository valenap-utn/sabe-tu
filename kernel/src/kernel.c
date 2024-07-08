#include"kernel.h"

    int conexion_memoria;
    int conexion_cpu_dispatch;
    int conexion_cpu_interrupt;
    t_list* interfaces;
    bool cambioDeProceso = true;

    pthread_mutex_t mutex_plani;

    sem_t contador_new;
    sem_t contador_ready;

    sem_t espacio_en_colas;

    t_dictionary* recursos;

    sem_t mutex_listas_recursos;


int main(int argc, char* argv[]){
    
    config = config_create("kernel.config");
    logger = log_create("Kernel.log","KERNEL",1,LOG_LEVEL_INFO);

    ready = list_create();
    new = list_create();
    blocked =list_create();
    interfaces = list_create();
    readyQuantum = list_create();
    recursos_por_proceso= list_create();


    sem_init(&contador_new,0,0);
    sem_init(&contador_ready,0,0);

    sem_init(&mutex_listas,0,1);
    sem_init(&mutex_listas_recursos,0,1);

    sem_init(&espacio_en_colas,0,config_get_int_value(config,"GRADO_MULTIPROGRAMACION"));


    pthread_mutex_init(&mutex_plani,NULL);
    iniciar_planificaciones();

    inicializar_recursos();

    pthread_t inter;

    pthread_create(&inter,NULL,(void*)controlar_interfaces,NULL);

    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA","memoria");
    conexion_cpu_dispatch = conectar("PUERTO_CPU_DISPATCH","IP_CPU","cpu dispatch");
    conexion_cpu_interrupt = conectar("PUERTO_CPU_INTERRUPT","IP_CPU","cpu interrupt");


    while(1)
    {
        char** comando = iniciarBash();
        switch (procesar_comando(comando[0]))
        {
        case INICIAR_PROCESO:
            PCB* pcb = iniciar_pcb(config_get_int_value(config,"QUANTUM"));
            paquete_a_memoria(comando[1],conexion_memoria,pcb->pid);
            list_add(new,pcb);
            log_info(logger,"Se crea el proceso <%d> en NEW",pcb->pid);
            
            inicializar_recursos_del_proceso(pcb);
            sem_post(&contador_new);

        break;
        case FINALIZAR_PROCESO:
            if(execute != NULL)if(execute->pid == atoi(comando[1])){
                exit_execute("INTERRUPTED_BY_USER");
                int i = 0;
			    send(conexion_cpu_interrupt,&i,sizeof(int),0);
            }
            sacarProcesoDeLista(ready,atoi(comando[1]));
            sacarProcesoDeLista(blocked,atoi(comando[1]));
            sacarProcesoDeLista(new,atoi(comando[1]));
            sem_post(&espacio_en_colas);
            log_info(logger,"Finaliza el proceso <%d> - Motivo: <INTERRUPTED_BY_USER>",atoi(comando[1]));
        break;
        case INICIAR_PLANIFICACION:
            pthread_mutex_unlock(&mutex_plani);
        break;
        case DETENER_PLANIFICACION:
            pthread_mutex_lock(&mutex_plani);
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


void sacarProcesoDeLista(t_list *lista,int i)
{
	sem_wait(&mutex_listas);
	int pid = i;
    bool cmpProcesoId(void *p){return ((PCB*)p)->pid == pid;}
	struct PCB* pcb = list_remove_by_condition(lista,cmpProcesoId);
	sem_post(&mutex_listas);
	if(pcb != NULL)
	{
        memoria_liberar_proceso(i);
		liberar_recursos(pcb);
		if(lista == ready)sem_wait(&contador_ready);
	}
}

void planFIFO()
{
    while(1)
    {
        sem_wait(&contador_ready);

        pthread_mutex_lock(&mutex_plani);
        pthread_mutex_unlock(&mutex_plani);


        sacarPrimerPCB();

        t_list* lista = enviar_al_CPU(execute);
        atender_syscall(lista);
    }
}

void planRR(void)
{
    while(1)
    {
        sem_wait(&contador_ready);

        pthread_mutex_lock(&mutex_plani);
        pthread_mutex_unlock(&mutex_plani);


        sacarPrimerPCB();

        pthread_t interrupciones;

        pthread_create(&interrupciones,NULL,(void*)interrupcionesRR,execute);

        t_list* lista = enviar_al_CPU(execute);
        atender_syscall(lista);
    }
}


void sacarPrimerPCB()
{
    if(cambioDeProceso)
    {
        sem_wait(&mutex_listas);
        execute = list_remove(ready,0);
        sem_post(&mutex_listas);
        cambioDeProceso = false;
        log_info(logger,"PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXECUTE>",execute->pid); 
    }
}

void planVRR()
{
    while(1)
    {

        sem_wait(&contador_ready);

        pthread_mutex_lock(&mutex_plani);
        pthread_mutex_unlock(&mutex_plani);
        if(cambioDeProceso)
        {
            if(list_is_empty(readyQuantum))
            {
                sem_wait(&mutex_listas);
                execute = list_remove(ready,0);
                sem_post(&mutex_listas);
            }
            else execute = list_remove(readyQuantum,0);

            cambioDeProceso = false;
        }

        pthread_t interrupciones;

        t_temporal* quantum = temporal_create();
        pthread_create(&interrupciones,NULL,(void*)interrupcionesRR,execute);


        t_list* lista = enviar_al_CPU(execute);
        atender_syscall(lista);

        if(cambioDeProceso)
        {
            execute->quantum -= temporal_gettime(quantum);
            
            sem_wait(&mutex_listas);
            list_remove_element(ready,execute);
            sem_post(&mutex_listas);

            list_add(readyQuantum,execute); //lo ponemos en la de mayor prioridad 
        }

        temporal_destroy(quantum);
    }
}


void planificacionLargoPlazo()
{
    while(1)
	{
		sem_wait(&espacio_en_colas);
		sem_wait(&contador_new);


		pthread_mutex_lock(&mutex_plani);
		pthread_mutex_unlock(&mutex_plani);

        sem_wait(&mutex_listas);
        struct PCB* pcb = list_remove(new,0);
        list_add(ready,pcb);
        sem_post(&mutex_listas);

        log_info(logger,"PID: <%d> - Estado Anterior: <NEW> - Estado Actual: <READY>",pcb->pid);

        loggear_lista(ready);
        sem_post(&contador_ready);
	}
}

void loggear_lista(t_list *lista)
{
    char* lista_string = string_new();
    string_append(&lista_string,"Cola Ready: [");
    for(int i = 0;i < list_size(lista);i++)
    {
        PCB* pcb = list_get(lista,i);
        string_append(&lista_string,string_itoa(pcb->pid));
        string_append(&lista_string,",");
    }
    string_append(&lista_string,"]");
    log_info(logger,"%s",lista_string);
}

void controlar_interfaces()
{
    int server_fd = iniciar_servidor();
    while(1)
    {
        int conexion_I_O = esperar_cliente(server_fd);
        {
        responder_handshake(conexion_I_O);
        }
        interfaz *i = malloc(sizeof(interfaz));
        i->conexion = conexion_I_O;
        i->libre = true;
        i->cola = list_create();
        sem_init(&i->trigger,0,0);
        recv(conexion_I_O,&i->tipo,sizeof(int),MSG_WAITALL);
        
        int tamanio;
        recv(conexion_I_O,&tamanio,sizeof(int),MSG_WAITALL);
        char nombre[tamanio+1];
        recv(conexion_I_O,nombre,tamanio,MSG_WAITALL);

        sprintf(nombre,"%.*s",tamanio,nombre);
        char* nombrePosta = string_new();
        string_append(&nombrePosta,nombre);

        i->nombre = nombrePosta;

        list_add(interfaces,i);

        pthread_t hilo;

        pthread_create(&hilo,NULL,(void*)operaciones_de_interfaz,i);

        pthread_detach(hilo);
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

                int parametro = (int)op->parametro;

                send(i->conexion,&parametro,sizeof(int),0);
            break;

            case IO_ESCRIBIR:
                comunicacion = IO_STDOUT_WRITE;
                send(i->conexion,&comunicacion,sizeof(int),0);
                

                send(i->conexion,list_remove((t_list*)op->parametro,0),sizeof(int),0);
                send(i->conexion,list_remove((t_list*)op->parametro,0),sizeof(int),0);
            break;
            case IO_LEER:
                comunicacion = IO_STDIN_READ;
                send(i->conexion,&comunicacion,sizeof(int),0);
                

                send(i->conexion,list_remove((t_list*)op->parametro,0),sizeof(int),0);
                send(i->conexion,list_remove((t_list*)op->parametro,0),sizeof(int),0);
            break;
            case -1:
                eliminar_interfaz(i);
            break;
            default:

            break;
        }
            recv(i->conexion,&comunicacion,sizeof(int),MSG_WAITALL);
            i->libre = true;
            desbloquearProceso(op->pcb);
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
	sem_post(&contador_ready);
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
    int i;
    recv(conexion_cpu_dispatch,&(paq),sizeof(bool),MSG_WAITALL);
    recv(conexion_cpu_dispatch,&i,sizeof(int),MSG_WAITALL);
    if(paq)return recibir_paquete(conexion_cpu_dispatch);
    return NULL;
}

void iniciar_planificaciones(void)
{
    pthread_t cortoPlazo;
    pthread_t largoPlazo;
    char *planificacion = string_new();
    string_append(&planificacion,config_get_string_value(config,"ALGORITMO_PLANIFICACION"));
    if(string_equals_ignore_case(planificacion,"fifo"))pthread_create(&cortoPlazo,NULL,(void*)planFIFO,NULL);
    if(string_equals_ignore_case(planificacion,"rr")) pthread_create(&cortoPlazo,NULL,(void*)planRR,NULL);
    if(string_equals_ignore_case(planificacion,"vrr")) pthread_create(&cortoPlazo,NULL,(void*)planVRR,NULL);
    pthread_create(&largoPlazo,NULL,(void*)planificacionLargoPlazo,NULL);

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
            cambioDeProceso = true;
		}
	}
}


void atender_syscall(t_list* lista)
{
    char* nombre;
    if(lista == NULL)
    {
        sem_wait(&mutex_listas);
        list_add(ready,execute);
        sem_post(&mutex_listas);
        return;
    }
    interfaz *i;
    switch(*(int*)list_get(lista,0))
    {
        case OUT_OF_MEMORY:
            exit_execute("OUT_OF_MEMORY");
        break;
        case SALIR:
            exit_execute("SUCCESS");
        break;
        case DORMIR:
            char *nombre_int = list_remove(lista,1);
            i = encontrarInterfaz(nombre_int,GENERICA);
            if(i != NULL)
            {
                struct cola_de_operaciones* op = malloc(sizeof(struct cola_de_operaciones));
                op->pcb = execute;
                op->parametro = *(int*)list_remove(lista,1);
                op->operacion = DORMIR;

                list_add(i->cola,op);

                bloquear_execute(nombre_int);

                sem_post(&i->trigger);
            }
        break;
        case ESPERAR:
        nombre = (char*)list_remove(lista,1);
		if(dictionary_has_key(recursos,nombre))
		{
			int instancias = (int)dictionary_get(recursos,nombre);
			dictionary_put(recursos,nombre,(void*)--instancias);

			sem_wait(&mutex_listas_recursos);
            rec* recursos_del_execute = (rec*)list_find(recursos_por_proceso,encontrar_recursos_del_execute);
			sem_post(&mutex_listas_recursos);

			log_info(logger,"PID: <%d> - Wait: <%s> - Instancias: <%d>",execute->pid,nombre,instancias);
			if(instancias < 0)
			{
                char* c = string_new();
                string_append(&c,nombre);
				recursos_del_execute->bloqueando = c;
				bloquear_execute(nombre);
			}
			else
			{
			 	int instanciasE = (int)dictionary_get(recursos_del_execute->recursos,nombre);
			 	dictionary_put(recursos_del_execute->recursos,nombre,(void*)++instanciasE);
                sem_post(&contador_ready);
			}
		}
		else{
			exit_execute("INVALID_RESOURCE");
		}
        break;
        case SENIAL:
        nombre = list_remove(lista,1);
        sem_wait(&mutex_listas_recursos);
        rec* recursos_del_execute = (rec*)list_find(recursos_por_proceso,encontrar_recursos_del_execute);
        sem_post(&mutex_listas_recursos);
		if((dictionary_has_key(recursos,nombre) && elProcesoTieneUnrecurso(recursos_del_execute,nombre)))
		{
			int instancias = dictionary_get(recursos,nombre);
			dictionary_put(recursos,nombre,++instancias);
			log_info(logger,"PID: <%d> - Signal: <%s> - Instancias: <%d>",execute->pid,nombre,instancias);

            int instanciasE =dictionary_get(recursos_del_execute->recursos,nombre);
            dictionary_put(recursos_del_execute->recursos,nombre,--instanciasE);

			if(instancias >= 0)
			{
				// desbloqueamos al primer proceso que pide el recurso
				liberar_procesos_bloqueados_por_recursos(nombre);
			}
            sem_post(&contador_ready);
		}
		else{
			memoria_liberar_proceso(execute->pid);
			exit_execute("INVALID_RESOURCE");
		}
        break;
        case IO_ESCRIBIR:
        { 
            char *nombre_int = list_remove(lista,1);
            i = encontrarInterfaz(nombre_int,GENERICA);
            if(i != NULL)
            {
                struct cola_de_operaciones* op = malloc(sizeof(struct cola_de_operaciones));
                op->pcb = execute;
                op->parametro = list_create();
                list_add((t_list*)op->parametro,list_remove(lista,1));
                list_add((t_list*)op->parametro,list_remove(lista,1));
                op->operacion = IO_ESCRIBIR;

                list_add(i->cola,op);

                bloquear_execute(nombre_int);

                sem_post(&i->trigger);
            }
        }
        break;
        case IO_LEER:
            {
                char *nombre_int = list_remove(lista,1);
                i = encontrarInterfaz(nombre_int,GENERICA);
                if(i != NULL)
                {
                    struct cola_de_operaciones* op = malloc(sizeof(struct cola_de_operaciones));
                    op->pcb = execute;
                    op->parametro = list_create();
                    list_add((t_list*)op->parametro,list_remove(lista,1));
                    list_add((t_list*)op->parametro,list_remove(lista,1));
                    op->operacion = IO_LEER;

                    list_add(i->cola,op);

                    bloquear_execute(nombre_int);

                    sem_post(&i->trigger);
                }
            }
        break;
    }
}

bool liberar_procesos_bloqueados_por_recursos(char* nombre)
{

    bool cmpRecurso(void * recurso)
    {
        if(((rec*)recurso)->bloqueando == NULL)return false;
        return string_equals_ignore_case(((rec*)recurso)->bloqueando,nombre);
    }

    rec* recu = list_find(recursos_por_proceso,cmpRecurso);

    if(recu != NULL)
    {
    free(recu->bloqueando);
    recu->bloqueando = NULL;
    int instanciasE =dictionary_get(recu->recursos,nombre);
    dictionary_put(recu->recursos,nombre,++instanciasE);

    desbloquearProceso(recu->pcb);
    return true;
    }
    return false;
}

bool elProcesoTieneUnrecurso(rec *recu ,char* nombre)
{
    return (int)dictionary_get(recu->recursos,nombre) > 0;
}
bool encontrar_recursos_del_execute(void *r)
{
    return ((rec*)r)->pcb == execute;
}

void bloquear_execute(char* nombre)
{
	sem_wait(&mutex_listas);
	list_add(blocked,execute);
	sem_post(&mutex_listas);
	log_info(logger,"PID: <%d> - Estado Anterior: <EXECUTE> - Estado Actual: <BLOCKED>",execute->pid);
	log_info(logger,"PID: <%d> - Bloqueado por: <%s>",execute->pid,nombre);
    cambioDeProceso = true;
}

interfaz* encontrarInterfaz(char* nombre,int tipo)
{   
    bool comparar(void* i)
    {
        return string_equals_ignore_case(((interfaz*)i)->nombre, nombre);
    };

    interfaz *i = list_find(interfaces,comparar);
    if(i == NULL ||  i->tipo != tipo)
    {
        exit_execute("INVALID_INTERFACE");
        return NULL;
    }

    return i;
}

void exit_execute(char * razon)
{
    	// desbloquear_todo(execute);
		// sacar_de_laMatriz(execute->PID);
        int pid = execute->pid;
        memoria_liberar_proceso(execute->pid);
        liberar_recursos(execute);
		free(execute);
		sem_post(&espacio_en_colas);
        log_info(logger,"Finaliza el proceso <%d> - Motivo: <%s>",pid,razon);
        cambioDeProceso = true;

}

void liberar_recursos(PCB* proceso)
{
    sem_wait(&mutex_listas_recursos);
    bool encontrar_recursos_del_proceso(void* r)
    {
        return ((rec*)r)->pcb == proceso;
    }
    rec* recurso_del_proceso = list_find(recursos_por_proceso,encontrar_recursos_del_proceso);
    for(int i = 0;i < dictionary_size(recurso_del_proceso->recursos);i++)
    {
        int instancias;
        char* nombre_del_rec = list_get(dictionary_keys(recurso_del_proceso->recursos),i);
        int cant = dictionary_get(recurso_del_proceso->recursos,nombre_del_rec);
        if(cant > 0)
        {
            if(liberar_procesos_bloqueados_por_recursos(nombre_del_rec))cant--;
            instancias = dictionary_get(recursos,nombre_del_rec);
            dictionary_put(recursos,nombre_del_rec,instancias + cant);
        }
    }
    if(recurso_del_proceso->bloqueando != NULL)
    {
        int instanacias_del_rec = dictionary_get(recursos,recurso_del_proceso->bloqueando);
        dictionary_put(recursos,recurso_del_proceso->bloqueando,++instanacias_del_rec);
    }
    list_remove_element(recursos_por_proceso,recurso_del_proceso);

    dictionary_destroy(recurso_del_proceso->recursos);
    free(recurso_del_proceso);
    sem_post(&mutex_listas_recursos);
}


void memoria_liberar_proceso(int pid)
{
	int i = FINALIZACION;

    send(conexion_memoria,&i,sizeof(int),0);
    send(conexion_memoria,&pid,sizeof(int),0);
}

void inicializar_recursos(void)
{
	recursos = dictionary_create();
	char** nombres_recursos = string_get_string_as_array(config_get_string_value(config,"RECURSOS"));
	char** instancias = string_get_string_as_array(config_get_string_value(config,"INSTANCIAS_RECURSOS"));
	for(int i=0; i < string_array_size(instancias);i++)
	{
		dictionary_put(recursos,nombres_recursos[i],atoi(instancias[i]));
	}
}

void inicializar_recursos_del_proceso(PCB* pcb)
{
    rec* r = malloc(sizeof(rec));
    r->pcb = pcb;
    r->bloqueando = NULL;
    t_dictionary* map_de_recursos = dictionary_create();
	char** nombres_recursos = string_get_string_as_array(config_get_string_value(config,"RECURSOS"));
    for(int i=0; i < string_array_size(nombres_recursos);i++)
	{
		dictionary_put(map_de_recursos,nombres_recursos[i],0);
	}
    r->recursos = map_de_recursos;
    list_add(recursos_por_proceso,r);
}