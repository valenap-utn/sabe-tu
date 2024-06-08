#include "memoria.h"

int pid;

int server_fd;

int main(int argc, char* argv[]) {
    
    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/memoria/memoria.config");
    logger = log_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/memoria/memoria.log","MEMORIA",1,LOG_LEVEL_INFO);

    procesos = list_create();

    server_fd = iniciar_servidor();

    int conexion_cpu = esperar_cliente(server_fd);
    responder_handshake(conexion_cpu);
    int conexion_kernel = esperar_cliente(server_fd);
    responder_handshake(conexion_kernel);

    pthread_create(&cpu,NULL,(void*)comunicacion_cpu,conexion_cpu);
    pthread_create(&kernel,NULL,(void*)comunicacion_kernel,conexion_kernel);
    pthread_create(&io,NULL,(void*)comunicacion_io,NULL);
    
    pthread_detach(cpu);
    pthread_join(kernel,NULL);
    return 0;
}

void comunicacion_cpu(int conexion)
{
    log_info(logger,"el hilo cpu");
    while(1)
    {
        int peticion;
        recv(conexion,&peticion,sizeof(int),MSG_WAITALL);
        switch (peticion)
        {
        case INSTRUCCION:
            int pc;

            recv(conexion,&pid,sizeof(int),MSG_WAITALL);
            recv(conexion,&pc,sizeof(int),MSG_WAITALL);

            proceso *p = (proceso*)list_find(procesos,cmpProcesoId);
            char* instruccion = string_new();
            string_append(&instruccion,list_get(p->instrucciones,pc));
            pc = string_length(instruccion);

            send(conexion,&pc,sizeof(int),0);
            send(conexion,instruccion,sizeof(char)*pc,0);
            free(instruccion);
        break;
        case MARCO:

        break;
        case ESCRITURA:

        break;
        case LECTURA:

        break;
        default:
            log_error(logger,"error en la comunicacion con el cpu");
            break;
        }
    }
}

bool cmpProcesoId(void *p){return ((proceso *)p)->pid == pid;}


void comunicacion_kernel(int conexion)
{
    log_info(logger,"el hilo kernel");
    while(1)
    {
        int peticion;
        recv(conexion,&peticion,sizeof(int),MSG_WAITALL);
        switch (peticion)
        {
        case CREACION:
            guardar_proceso(conexion);
        break;
        case FINALIZACION:
            
            recv(conexion,&pid,sizeof(int),MSG_WAITALL);
        
            proceso *p = list_remove_by_condition(procesos,cmpProcesoId);

            list_destroy_and_destroy_elements(p->instrucciones,free);
            free(p->nombre);
            free(p);
        break;
        case AJUSTAR:

        break;
        default:
            log_error(logger,"error en la comunicacion con el kernel");
            break;
        }
    }
}

void comunicacion_io(void)
{
    int conexion = esperar_cliente(server_fd);
    responder_handshake(conexion);
    log_info(logger,"el hilo entreada/salida");
}

struct proceso *guardar_proceso(int conexion)
{
    int tamanio;
    proceso* p = malloc(sizeof(proceso));
    recv(conexion,&tamanio,sizeof(int),MSG_WAITALL);
    p->instrucciones = list_create();
    p->nombre = malloc(sizeof(char)*tamanio);
    recv(conexion,p->nombre,sizeof(char)*tamanio,MSG_WAITALL);
    recv(conexion,&p->pid,sizeof(int),MSG_WAITALL);
	char *linea = malloc(sizeof(char[50]));
	char* c = string_new();
	string_append(&c,config_get_string_value(config,"PATH_INSTRUCCIONES"));
	string_append(&c,p->nombre);
	FILE *archivo = fopen(c, "r");
	size_t len = 0;
	ssize_t read;
 	while((read = getline(&linea, &len, archivo)) != -1){
        list_add(p->instrucciones,linea);
		linea = malloc(sizeof(char[50]));
	}
	list_add(procesos,p);
	fclose(archivo);
	return p;
}