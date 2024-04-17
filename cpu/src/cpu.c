#include "cpu.h"



int main(int argc, char* argv[]) {
    
    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/cpu/cpu.config");
    logger = log_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/cpu/cpu.log","CPU",1,LOG_LEVEL_INFO);

    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA");
    
    pthread_create(&interrupciones,NULL,(void*)interrupt,NULL);
    pthread_create(&dispatch,NULL,(void*)instrucciones,NULL);

    pthread_join(interrupciones,NULL);
    pthread_join(dispatch,NULL);
}

void interrupt()
{
    int server_fd = iniciar_servidor("PUERTO_ESCUCHA_INTERRUPT");
	int conexion_kernel = esperar_cliente(server_fd);
    responder_handshake(conexion_kernel);

    while(1)
    {
        recv(conexion_kernel,&interrupcion,sizeof(int),MSG_WAITALL);
    }
}

void instrucciones()
{
    int server_fd = iniciar_servidor("PUERTO_ESCUCHA_DISPATCH");
	int conexion_kernel = esperar_cliente(server_fd);
    responder_handshake(conexion_kernel);

    while(1)
    {
        actualizar_registros(conexion_kernel);
        while(interrupcion)
        {
            char* instruccion = fetch();
            t_list * ins = decode(instruccion);
            execute(ins);
        }
        devolver_contexto(conexion_kernel);
    }
}


void actualizar_registros(int conexion)
{
    recv(conexion,&PID,sizeof(int),MSG_WAITALL);
    recv(conexion,&PC,sizeof(uint32_t),MSG_WAITALL);
    recv(conexion,&AX,sizeof(uint8_t),MSG_WAITALL);
    recv(conexion,&BX,sizeof(uint8_t),MSG_WAITALL);
    recv(conexion,&CX,sizeof(uint8_t),MSG_WAITALL);
    recv(conexion,&DX,sizeof(uint8_t),MSG_WAITALL);
    recv(conexion,&EAX,sizeof(uint32_t),MSG_WAITALL);
    recv(conexion,&EBX,sizeof(uint32_t),MSG_WAITALL);
    recv(conexion,&ECX,sizeof(uint32_t),MSG_WAITALL);
    recv(conexion,&EDX,sizeof(uint32_t),MSG_WAITALL);
    recv(conexion,&SI,sizeof(uint32_t),MSG_WAITALL);
    recv(conexion,&DI,sizeof(uint32_t),MSG_WAITALL);
}

void devolver_contexto(int conexion)
{
    send(conexion,&PC,sizeof(uint32_t),0);
    send(conexion,&AX,sizeof(uint8_t),0);
    send(conexion,&BX,sizeof(uint8_t),0);
    send(conexion,&CX,sizeof(uint8_t),0);
    send(conexion,&DX,sizeof(uint8_t),0);
    send(conexion,&EAX,sizeof(uint32_t),0);
    send(conexion,&EBX,sizeof(uint32_t),0);
    send(conexion,&ECX,sizeof(uint32_t),0);
    send(conexion,&EDX,sizeof(uint32_t),0);
    send(conexion,&SI,sizeof(uint32_t),0);
    send(conexion,&DI,sizeof(uint32_t),0);
}

// char* fetch()
// {

// }