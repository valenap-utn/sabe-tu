#include "memoria.h"

int main(int argc, char* argv[]) {
    
    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/memoria/memoria.config");
    logger = log_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/memoria/memoria.log","MEMORIA",1,LOG_LEVEL_INFO);



    int server_fd = iniciar_servidor();

    int conexion_cpu = esperar_cliente(server_fd);
    responder_handshake(conexion_cpu);
    int conexion_kernel = esperar_cliente(server_fd);
    responder_handshake(conexion_kernel);
    int conexion_io = esperar_cliente(server_fd);
    responder_handshake(conexion_io);

    pthread_create(&cpu,NULL,(void*)comunicacion_cpu,&conexion_cpu);
    pthread_create(&kernel,NULL,(void*)comunicacion_kernel,&conexion_kernel);
    pthread_create(&io,NULL,(void*)comunicacion_io,&conexion_io);
    return 0;
}

void comunicacion_cpu(int conexion)
{
    log_info(logger,"el hilo cpu");
}

void comunicacion_kernel(int conexion)
{
    log_info(logger,"el hilo kernel");
}

void comunicacion_io(int conexion)
{
    log_info(logger,"el hilo entreada/salida");
}