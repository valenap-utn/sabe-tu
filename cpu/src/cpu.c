#include "cpu.h"



int main(int argc, char* argv[]) {
    
    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/cpu/cpu.config");
    logger = log_create("cpu/cpu.log","CPU",1,LOG_LEVEL_INFO);

    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA");
    
    pthread_create(&interrupciones,NULL,interrupt,NULL);
    pthread_create(&dispatch,NULL,instrucciones,NULL);
}

void interrupt()
{
    int server_fd = iniciar_servidor();
	int conexion_kernel = esperar_cliente(server_fd);
    responder_handshake(conexion_kernel);
}

void instrucciones()
{
    int server_fd = iniciar_servidor();
	int conexion_kernel = esperar_cliente(server_fd);
    responder_handshake(conexion_kernel);
}