#include "entradasalida.h"


int main(int argc, char* argv[]) {
    int conexion_memoria;

    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/entradasalida/io.config");
    logger = log_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/entradasalida/entradasalida.log","ENTRADASALIDA",1,LOG_LEVEL_INFO);


    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA",config);
    
    // se crea una interfaz
    
    // PREGUNTAS (para no olvidarnos)
    // Que es lo que conecta las I/O con el Kernel?
    // Como se inician las I/O? Se inician? ahre 

    interfaz i;
    
	while (1) {
   	pthread_t thread;

  	pthread_create(&thread,
                  NULL,
                  (void*) inter,
                  &i);
	pthread_detach(thread);
	}

    return 0;
}

void inter(interfaz *inter)
{
    int conexion_kernel = conectar("PUERTO_KERNEL","IP_KERNEL",&inter->configuracion);
    int tipo = tipoInter(config_get_string_value(&inter->configuracion,"TIPO_INTERFAZ"));
    send(conexion_kernel,&tipo,sizeof(int),0);
    send(conexion_kernel,inter->nombre,sizeof(char)*20,0);
    int peticion;
    recv(conexion_kernel,&peticion,sizeof(int),MSG_WAITALL);
    switch(peticion)
    {
        case IO_GEN_SLEEP:
            int cantidad;
            recv(conexion_kernel,&cantidad,sizeof(int),MSG_WAITALL);
            usleep(config_get_int_value(&inter->configuracion,"TIEMPO_UNIDAD_TRABAJO")*cantidad*1000);

            send(conexion_kernel,&cantidad,sizeof(int),0);
        break;
        case IO_STDIN_READ:
        
        break;
        case IO_STDOUT_WRITE:
        

        break;
        case IO_FS_TRUNCATE:
        
        break;
        case IO_FS_CREATE:
        
        break;
        case IO_FS_DELETE:
        
        break;
        case IO_FS_READ:
        
        break;
        case IO_FS_WRITE:
        
        break;
    }
}

int tipoInter(char* s)
{
    char *c = string_new();
    string_append(&c,s);
    if(string_equals_ignore_case(c,"GenericaS"))return GENERICA;
    if(string_equals_ignore_case(c,"STDIN"))return STDIN;
    if(string_equals_ignore_case(c,"STDOUT"))return STDOUT;
    if(string_equals_ignore_case(c,"DialFS"))return DIALFS;
    return -1;
}