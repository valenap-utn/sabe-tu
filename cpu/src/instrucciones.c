#include "instrucciones.h"


void set(void* registro, int valor)
{
    *(int*)registro = valor;
}

void mov_in(void* registro_datos,void* registro_direccion)
{
    // despues
}

void mov_out(void* registro_datos,void* registro_direccion)
{
    // despues
}

void sum(void* destino,void* origen)
{
    *(int*)destino += *(int*)origen;
}

void sub(void* destino,void* origen)
{
    *(int*)destino -= *(int*)origen;
}

void jnz(void* registro,uint32_t ins)
{
    if(*(int*)registro == 0)PC = ins;
}

void resize(int tamanio)
{
    //despues
}

void copy_string(void* tamanio)
{
    //despues
}

void wait(void* recurso)
{
    //
}

void s1gnal(void* recurso)
{
    //
}

void io_gen_sleep(char* interfaz,int unidad_trabajo)
{
    int i = DORMIR;
    paquete = crear_paquete();
    agregar_a_paquete(paquete,&i,sizeof(int));
    agregar_a_paquete(paquete,interfaz,string_length(interfaz));
    agregar_a_paquete(paquete,(void*)unidad_trabajo,sizeof(int));
    sysCall = false;
}

void io_stdin_read(char* interfaz,void* registro_direccion,void* registro_tamanio)
{

}

void io_stdout_write(char* interfaz,void* registro_direccion,void* registro_tamanio)
{
    //
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
    //
}


