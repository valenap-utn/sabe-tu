#include "entradasalida.h"


int main(int argc, char* argv[]){
    
    int conexion_memoria;
    
    char *con = string_new();
    string_append(&con,"entradasalida/");
    string_append(&con, argv[0]);
    string_append(&con,".config");
    config = config_create(con);
    char *log = string_new();
    string_append(&log,"entradasalida/");
    string_append(&log,argv[0]);
    logger = log_create(log,"ENTRADASALIDA",1,LOG_LEVEL_INFO);


    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA",config);
    
    // se crea una interfaz
    inter(argv[0]);
    return 0;
}

void inter(char *nombre)
{
    char* nombreBis = string_new();
    string_append(&nombreBis,nombre);
    int conexion_kernel = conectar("PUERTO_KERNEL","IP_KERNEL",config);
    handshake(conexion_kernel);
    int tipo = tipoInter(config_get_string_value(config,"TIPO_INTERFAZ"));
    int largoNombre = string_length(nombreBis);
    send(conexion_kernel,&tipo,sizeof(int),0);
    send(conexion_kernel,&largoNombre,sizeof(int),0);
    send(conexion_kernel,nombre,sizeof(char)*20,0);
    int peticion;
    
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