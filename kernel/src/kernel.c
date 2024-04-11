#include"kernel.h"

    int conexion_memoria;
    int conexion_I_O;
    int conexion_cpu_dispatch;
    int conexion_cpu_interrupt;

int main(int argc, char* argv[]) {
    
    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/kernel/kernel.config");
    logger = log_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/kernel/Kernel.log","KERNEL",1,LOG_LEVEL_INFO);


    int server_fd = iniciar_servidor();
    log_info(logger,"hola soy el kernel y funciono");
	conexion_I_O = esperar_cliente(server_fd);
    {
        int i;
        recv(conexion_I_O,&i,sizeof(int),MSG_WAITALL);
        send(conexion_I_O,&i,sizeof(int),0);
    }


    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA","memoria");
    conexion_cpu_dispatch = conectar("PUERTO_CPU_DISPATCH","IP_CPU","cpu dispatch");
    conexion_cpu_interrupt = conectar("PUERTO_CPU_INTERRUPT","IP_CPU","cpu interrupt");
    log_info(logger, "Entrada y salida conectada");
    return 0;
}


