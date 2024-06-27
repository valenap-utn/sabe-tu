#include "entradasalida.h"

int conexion_memoria;
int main(int argc, char* argv[]){
    
    char *con = string_new();
    // string_append(&con, "entradasalida/");
    string_append(&con, argv[1]);
    string_append(&con,".config");
    config = config_create(con);
    char *log = string_new();
    // string_append(&log, "entradasalida/");
    string_append(&log,argv[1]);
    string_append(&log,".log");
    logger = log_create(log,"ENTRADASALIDA",1,LOG_LEVEL_INFO);

    log_info(logger,"%s",config_get_string_value(config,"IP_MEMORIA"));
    log_info(logger,"%s",config_get_string_value(config,"PUERTO_MEMORIA"));

    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA");
    
    // se crea una interfaz
    inter(argv[1]);
    return 0;
}

void inter(char *nombre)
{
    char* nombreBis = string_new();
    string_append(&nombreBis,nombre);
    string_append(&nombreBis,"\0");
    int conexion_kernel = conectar("PUERTO_KERNEL","IP_KERNEL");
    int tipo = tipoInter(config_get_string_value(config,"TIPO_INTERFAZ"));
    int largoNombre = string_length(nombreBis);
    send(conexion_kernel,&tipo,sizeof(int),0);
    send(conexion_kernel,&largoNombre,sizeof(int),0);
    send(conexion_kernel,nombre,sizeof(char)*largoNombre,0);
    int peticion;
    int tamanio;
    int direccion;
    int comunicacion;
    int pid;
    char* output,*input;
    while(1)
    {
        recv(conexion_kernel,&peticion,sizeof(int),MSG_WAITALL);
        switch(peticion)
        {
            case IO_GEN_SLEEP:
                int cantidad;
                recv(conexion_kernel,&cantidad,sizeof(int),MSG_WAITALL);
                usleep(config_get_int_value(config,"TIEMPO_UNIDAD_TRABAJO")*cantidad*1000);

                send(conexion_kernel,&cantidad,sizeof(int),0);
            break;
            case IO_STDOUT_WRITE:
                recv(conexion_kernel,&direccion,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);
                input = malloc(sizeof(char)*tamanio);
                for(int i = 0;i < tamanio;i++)input[i] = getchar();
                comunicacion = ESCRIBIR;
                send(conexion_memoria,&comunicacion,sizeof(int),0);
                
                send(conexion_memoria,&direccion,sizeof(int),0);
                send(conexion_memoria,&tamanio,sizeof(int),0);
                send(conexion_memoria,&pid,sizeof(int),0);
                send(conexion_memoria,input,tamanio,0);

                recv(conexion_memoria,&comunicacion,sizeof(int),MSG_WAITALL);
                free(input);
            break;
            case IO_STDIN_READ:
                recv(conexion_kernel,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&direccion,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);
                
                
                comunicacion = LEER;
                send(conexion_memoria,&comunicacion,sizeof(int),0);
                send(conexion_memoria,&direccion,sizeof(int),0);
                send(conexion_memoria,&tamanio,sizeof(int),0);
                send(conexion_memoria,&pid,sizeof(int),0);
                output = malloc(sizeof(char)*tamanio);
                recv(conexion_memoria,output,tamanio,MSG_WAITALL);

                printf("%s",output);
                free(output);
            break;
            case IO_FS_TRUNCATE:
            
            break;
            case IO_FS_CREATE:
            
            break;
            case IO_FS_DELETE:
            
            break;

        }
    }
}

int tipoInter(char* s)
{
    char *c = string_new();
    string_append(&c,s);
    if(string_equals_ignore_case(c,"Generica"))return GENERICA;
    if(string_equals_ignore_case(c,"STDIN"))return STDIN;
    if(string_equals_ignore_case(c,"STDOUT"))return STDOUT;
    if(string_equals_ignore_case(c,"DialFS"))return DIALFS;
    return -1;
}