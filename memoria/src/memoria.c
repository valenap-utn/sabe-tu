#include "memoria.h"

void* espacioDeUsuario;

//algo tabla de paginas


int tamanio;
int cant_paginas;
int tamanio_pags;
int paginasLibres;

bool *paginasOcupadas;

int pid;

int server_fd;

int main(int argc, char* argv[]) {
    
    config = config_create("memoria.config");
    logger = log_create("memoria.log","MEMORIA",1,LOG_LEVEL_INFO);

    procesos = list_create();

    tamanio = config_get_int_value(config,"TAM_MEMORIA");

    tamanio_pags = config_get_int_value(config,"TAM_PAGINA");

    cant_paginas = tamanio/tamanio_pags;

    paginasLibres =cant_paginas;

    iniciar_paginasOcupadas();

    pthread_mutex_init(&mutex_pid,NULL);



    espacioDeUsuario = malloc(sizeof(char)*tamanio);

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
        usleep(config_get_int_value(config,"RETARDO_RESPUESTA")*1000);
        proceso *p;
        switch (peticion)
        {
            case INSTRUCCION:
            {
                int pc;

                pthread_mutex_lock(&mutex_pid);
                recv(conexion,&pid,sizeof(int),MSG_WAITALL);
                recv(conexion,&pc,sizeof(int),MSG_WAITALL);

                p = (proceso*)list_find(procesos,cmpProcesoId);
                pthread_mutex_unlock(&mutex_pid);
                
                char* instruccion = string_new();
                string_append(&instruccion,list_get(p->instrucciones,pc));
                pc = string_length(instruccion);


                send(conexion,&pc,sizeof(int),0);
                send(conexion,instruccion,sizeof(char)*pc,0);
                free(instruccion);
            }    
            break;
            case MARCO:
            {
                uint32_t pagina;

                pthread_mutex_lock(&mutex_pid);
                recv(conexion,&pid,sizeof(int),MSG_WAITALL);
                recv(conexion,&pagina,sizeof(uint32_t),MSG_WAITALL);

                p = (proceso*)list_find(procesos,cmpProcesoId);

                int marco = (int)list_get(p->paginas,pagina);

                log_info(logger,"PID: <%d> - Pagina: <%u> - Marco: <%d>",p->pid,pagina,marco);
                pthread_mutex_unlock(&mutex_pid);
                send(conexion,&marco,sizeof(int),0);
            }
            break;
            case ESCRITURA:
            {   
                uint32_t direccion;
                int tamanio;
                int pid;
                uint32_t mensaje;
                recv(conexion,&direccion,sizeof(int),MSG_WAITALL);
                recv(conexion,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion,&pid,sizeof(int),MSG_WAITALL);

                recv(conexion,&mensaje,tamanio,MSG_WAITALL);
                char *c = uint32_to_bytes(mensaje);
                recibir_escritura(conexion,direccion,tamanio,pid,c);
                
                log_info(logger,"PID: <%d> - Accion: <ESCRIBIR> - Direccion fisica: <%u> - Tamaño <%d>", pid,direccion,tamanio);
                int confirmacion = 1;
                send(conexion,&confirmacion,sizeof(int),0);

                free(c);
            }
            break;
            case LECTURA:
            {
                int direccion;
                int tamanio = 1;
                int pid;
                recv(conexion,&direccion,sizeof(int),MSG_WAITALL);
                recv(conexion,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion,&pid,sizeof(int),MSG_WAITALL);
        
                log_info(logger, "PID: <%d> - Accion: <LEER> - Direccion fisica: <%d> - Tamaño <%d>", pid, direccion, tamanio);
                char *c = leer_peticion(pid,direccion,tamanio);
                uint32_t* respuesta = bytes_to_uint32(c);

                send(conexion,respuesta,tamanio,0);
                free(c);
            }
            break;
            case AJUSTE:
            {
                int new_tam;
                pthread_mutex_lock(&mutex_pid);
                recv(conexion,&pid,sizeof(int),MSG_WAITALL);
                recv(conexion,&new_tam,sizeof(int),MSG_WAITALL);

                p = (proceso*)list_find(procesos,cmpProcesoId);
                pthread_mutex_unlock(&mutex_pid);  
                
                bool exito = modificar_paginas_proceso(p,new_tam);

                send(conexion,&exito,sizeof(bool),0);
            }    
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
        usleep(config_get_int_value(config,"RETARDO_RESPUESTA")*1000);
        switch (peticion)
        {
            case CREACION:
            {
                guardar_proceso(conexion);

                send(conexion,&peticion,sizeof(int),0);
            }    
            break;
            case FINALIZACION:
            {    
                pthread_mutex_lock(&mutex_pid);
                recv(conexion,&pid,sizeof(int),MSG_WAITALL);
            
                proceso *p = list_remove_by_condition(procesos,cmpProcesoId);
                
                log_info(logger,"PID: <%d> - Tamaño: <%d>",p->pid,p->tamanio/tamanio_pags);
                pthread_mutex_unlock(&mutex_pid);

                modificar_paginas_proceso(p,0);

                list_destroy_and_destroy_elements(p->instrucciones,free);
                free(p->nombre);
                free(p);
            }    
            break;
            default:
                log_error(logger,"error en la comunicacion con el kernel");
                break;
        }
    }
}

void comunicacion_io(void)
{
    while(1)
    {
        int conexion = esperar_cliente(server_fd);
        responder_handshake(conexion);
        pthread_t hilo;
        pthread_create(&hilo,NULL,atender_io,conexion);
    }
}

void atender_io(int conexion)
{
    while(1)
    {
        int peticion;
        recv(conexion,&peticion,sizeof(int),MSG_WAITALL);
        usleep(config_get_int_value(config,"RETARDO_RESPUESTA")*1000);
        switch(peticion){
            case ESCRIBIR:
            {
                uint32_t direccion;
                int tamanio;
                int pid;
                char traduccion;
                recv(conexion,&direccion,sizeof(int),MSG_WAITALL);
                recv(conexion,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion,&pid,sizeof(int),MSG_WAITALL);

                char mensaje[tamanio];

                recv(conexion,mensaje,tamanio,MSG_WAITALL);
                recibir_escritura(conexion,direccion,tamanio,pid,mensaje);
                
                log_info(logger,"PID: <%d> - Accion: <ESCRIBIR> - Direccion fisica: <%u> - Tamaño <%d>", pid,direccion,tamanio);
                int confirmacion = 1;
                send(conexion,&confirmacion,sizeof(int),0);
            }
            break; 
            case LEER:
            {
                int direccion;
                int tamanio;
                int pid;
                recv(conexion,&direccion,sizeof(int),MSG_WAITALL);
                recv(conexion,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion,&pid,sizeof(int),MSG_WAITALL);
        
                log_info(logger, "PID: <%d> - Accion: <LEER> - Direccion fisica: <%d> - Tamaño <%d>", pid, direccion, tamanio);

                char* respuesta = leer_peticion(pid,direccion,tamanio);

                send(conexion,respuesta,tamanio,0);

                free(respuesta);
            }
            break;
            
            case -1:
                
            break;
        }
    }
}

struct proceso *guardar_proceso(int conexion)
{
    int tamanio;
    proceso* p = malloc(sizeof(proceso));
    recv(conexion,&tamanio,sizeof(int),MSG_WAITALL);
    p->instrucciones = list_create();
    p->nombre = malloc(sizeof(char)*tamanio);
    p->tamanio = 0;
    p->paginas = list_create();
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
    log_info(logger,"PID: <%d> - Tamaño: <0>",p->pid);
	return p;
}

void iniciar_paginasOcupadas()
{
    paginasOcupadas = malloc(sizeof(char)*cant_paginas);
    for(int i = 0;i < cant_paginas;i++)((char*)paginasOcupadas)[i] = false;
}

char* uint32_to_bytes(uint32_t valor) {
    char *bytes = malloc(sizeof(char)*4);
    bytes[0] = (valor >> 24) & 0xFF; // Byte más significativo
    bytes[1] = (valor >> 16) & 0xFF;
    bytes[2] = (valor >> 8) & 0xFF;
    bytes[3] = valor & 0xFF; // Byte menos significativo
    return bytes;
}

uint32_t bytes_to_uint32(const unsigned char bytes[4]) {
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) | bytes[3];
}

bool modificar_paginas_proceso(proceso* p,int new_tam)
{   //Verificamos si hay espacio
    if((new_tam - p->tamanio)/tamanio_pags < paginasLibres)
    {
        return false;
    }
    //Ampliamos
    if(p->tamanio < new_tam)
    {
        log_info(logger,"PID: <%d> - Tamaño Actual: <%d> - Tamaño a Ampliar: <%d>",p->pid,p->tamanio,new_tam);
        for(int i = 0;(i < cant_paginas) && p->tamanio < new_tam;i++)
        {
            if(!paginasOcupadas[i])
            {
                list_add(p->paginas,i);
                paginasOcupadas[i] = true;
                paginasLibres--;
                p->tamanio+= tamanio_pags;
            }
        }
    }
    else //O reducimos 
    {
        log_info(logger,"PID: <%d> - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>",p->pid,p->tamanio,new_tam);
        for(int i = list_size(p->paginas);p->tamanio != new_tam ;i--)
        {
            paginasOcupadas[(int)list_remove(p->paginas,i)] = false;
            paginasLibres++;
            p->tamanio -= tamanio_pags;

        }
    }
    return true;
}


void recibir_escritura(int conexion,int direccion,int tamanio,int pid,char* mensaje)
{
    int marco = floor(direccion/tamanio_pags);
    int espacio_libre = tamanio_pags -  direccion % tamanio_pags;
    int indice = 0;
    while(tamanio != 0)
    {   
        int minimo = min(espacio_libre,tamanio);
        for(int i = 0;i < minimo; i++)
        {
            ((char*)espacioDeUsuario)[direccion+i] = mensaje[indice++];
        }
        marco = proximo_marco(pid,marco);
        tamanio -= minimo;
        espacio_libre = tamanio_pags;
        direccion = marco*tamanio_pags;
    }
}

void* leer_peticion(int pid,int direccion,int tamanio)
{
    int marco = floor(direccion/tamanio_pags);
    int espacio_libre = tamanio_pags -  direccion % tamanio_pags;
    char* lectura = malloc(sizeof(char)*tamanio);
    int indice = 0;
    while(tamanio != 0)
    {   
        int minimo = min(espacio_libre,tamanio);
        for(int i = 0;i < minimo; i++)
        {
            lectura[indice++] = ((char*)espacioDeUsuario)[direccion+i];
        }
        marco = proximo_marco(pid,marco);
        tamanio -= minimo;
        espacio_libre = tamanio_pags;
        direccion = marco*tamanio_pags;
    }
    return lectura;
}

int proximo_marco(int Pid, int actual)
{
    pthread_mutex_lock(&mutex_pid);
    pid = Pid; 

    proceso *p = list_find(procesos,cmpProcesoId);

    pthread_mutex_unlock(&mutex_pid);
    
    int i = 0;
    while((int)list_get(p->paginas,i) != actual)i++;
    return (int)list_get(p->paginas,++i);
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}
