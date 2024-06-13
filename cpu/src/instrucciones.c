#include "instrucciones.h"

int sCall;
void set(void* registro, uint32_t valor)
{
    *(uint32_t*)registro = valor;
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
    sCall = DORMIR;
    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    agregar_a_paquete(paquete,interfaz,string_length(interfaz));
    agregar_a_paquete(paquete,&unidad_trabajo,sizeof(int));
    free(interfaz);
    sysCall = true;
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
    sCall = SALIR;
    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    sysCall = true;
}


