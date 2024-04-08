#include "entradasalida.h"


int main(int argc, char* argv[]) {
    int conexion_kernel;
    int conexion_memoria;

    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/entradasalida/entradasalida.config");
    logger = log_create("entradasalida/entradasalida.log","ENTRADASALIDA",1,LOG_LEVEL_INFO);

    conexion_kernel = conectar("PUERTO_KERNEL","IP_KERNEL");
    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA");

    return 0;
}
