#include "instrucciones.h"

int sCall;

char* nombre;
void set(void* registro, uint32_t valor)
{
    *(uint32_t*)registro = valor;
}

void mov_in(void* registro_datos,void* registro_direccion)
{
    int comu = LECTURA;
    send(conexion_memoria,&comu,sizeof(int),0);
    send(conexion_memoria,registro_direccion,sizeof(int),0);
    send(conexion_memoria,&PID,sizeof(int),0);
    int tam = tamanio(registro_direccion);
    send(conexion_memoria,&tam,sizeof(int),0);
    

    recv(conexion_memoria,(uint32_t)registro_datos,tam,MSG_WAITALL);
}

void mov_out(void* registro_datos,void* registro_direccion)
{
    int comu = ESCRITURA;
    send(conexion_memoria,&comu,sizeof(int),0);
    send(conexion_memoria,registro_direccion,sizeof(int),0);
    send(conexion_memoria,&PID,sizeof(int),0);
    int tam = tamanio(registro_direccion);
    send(conexion_memoria,&tam,sizeof(int),0);
    send (conexion_memoria,registro_datos,tam,0);

    recv(conexion_memoria,&comu,sizeof(int),MSG_WAITALL);
}

void sum(void* destino,void* origen)
{
    *(uint32_t*)destino += *(uint32_t*)origen;
}

void sub(void* destino,void* origen)
{
    *(uint32_t*)destino -= *(uint32_t*)origen;
}

void jnz(void* registro,uint32_t ins)
{
    if(*(uint32_t*)registro == 0)PC = ins;
}

void resize(int tamanio)
{
    int comu = AJUSTE;
    send(conexion_memoria,&comu,sizeof(int),0);
    send(conexion_memoria,&PID,sizeof(int),0);
    send(conexion_memoria,&tamanio,sizeof(int),0);

    bool exito;
    recv(conexion_memoria,&exito,sizeof(bool),MSG_WAITALL);
    if(!exito)  
    {
        sCall = OUT_OF_MEMORY;
        paquete = crear_paquete();
        agregar_a_paquete(paquete,&sCall,sizeof(int));
        sysCall = true;
    }
}

void copy_string(void* tamanio)
{
    int comu = COPIAR;
    send(conexion_memoria,&comu,sizeof(int),0);
    send(conexion_memoria,&PID,sizeof(int),0);
    send(conexion_memoria,&DI,sizeof(int),0);
    send(conexion_memoria,&tamanio,sizeof(int),0);
    send(conexion_memoria,&SI,tamanio,0);

    recv(conexion_memoria,&comu,sizeof(int),MSG_WAITALL);       
}

void wait(char* recurso)
{
    sCall = ESPERAR;
    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    nombre = recurso;
    agregar_a_paquete(paquete,nombre,string_length(nombre));
    free(recurso);
    sysCall = true;
}

void s1gnal(char* recurso)
{
    sCall = SENIAL;
    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    nombre = recurso;
    agregar_a_paquete(paquete,nombre,string_length(nombre));
    free(recurso);
    sysCall = true;
}

void io_gen_sleep(char* interfaz,int unidad_trabajo)
{
    sCall = DORMIR;
    paquete = crear_paquete();
    nombre = interfaz;
    agregar_a_paquete(paquete,nombre,string_length(nombre));
    agregar_a_paquete(paquete,&unidad_trabajo,sizeof(int));
    free(interfaz);
    sysCall = true;
}

void io_stdin_read(char* interfaz,void* registro_direccion,void* registro_tamanio)
{
    sCall = IO_LEER;
    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    nombre = interfaz;
    agregar_a_paquete(paquete,nombre,string_length(nombre));
    agregar_a_paquete(paquete,(int*)registro_direccion,sizeof(int));
    agregar_a_paquete(paquete,(int*)registro_tamanio,sizeof(int));
    free(interfaz);
    sysCall = true;
}

void io_stdout_write(char* interfaz,void* registro_direccion,void* registro_tamanio)
{
    sCall = IO_ESCRIBIR;
    paquete = crear_paquete();
    nombre = interfaz;
    agregar_a_paquete(paquete,nombre,string_length(nombre));
    agregar_a_paquete(paquete,(int*)registro_direccion,sizeof(int));
    agregar_a_paquete(paquete,(int*)registro_tamanio,sizeof(int));
    free(interfaz);
    sysCall = true;
}

void io_fs_create(char* interfaz,void* nombre_archivo)
{
    //
}

void io_fs_delete(char* interfaz,void* nombre_archivo)
{
    //
}

void io_fs_truncate(char* interfaz,void* nombre_archivo,void* registro_tamanio)
{
    //
}

void io_fs_write(char* interfaz,void* nombre_archivo,void* registro_direccion,void* registro_tamanio,void* registro_puntero_archivo)
{
    //
}

void io_fs_read(char* interfaz,void* nombre_archivo,void* registro_direccion,void* registro_tamanio,void* registro_puntero_archivo)
{
    //
}

void salir()
{
    sCall = SALIR;
    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    sysCall = true;
}


int tamanio(void* registro)
{
    if(registro == &AX)return sizeof(uint8_t);
    if(registro== &BX)return sizeof(uint8_t);
    if(registro == &CX)return sizeof(uint8_t);
    if(registro == &DX)return sizeof(uint8_t);
    return sizeof(uint32_t);
}