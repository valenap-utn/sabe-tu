#include "instrucciones.h"

int sCall;
int tam_pagina;
char* nombre;
char* nombre2;

void set(void* registro, uint32_t valor)
{
    if(tamanio(registro) == sizeof(uint8_t))*(uint8_t*)registro = valor;
    else *(uint32_t*)registro = valor;
}

void mov_in(void* registro_datos,void* registro_direccion)
{
    uint32_t direccion_fisica;
    if(tamanio(registro_direccion) == sizeof(uint8_t))direccion_fisica = traducir_direccion(*(uint8_t*)registro_direccion);
    else direccion_fisica = traducir_direccion(*(uint32_t*)registro_direccion);
    
    int comu = LECTURA;
    send(conexion_memoria,&comu,sizeof(int),0);
    send(conexion_memoria,&direccion_fisica,sizeof(int),0);
    int tam = tamanio(registro_datos);
    send(conexion_memoria,&tam,sizeof(int),0);
    send(conexion_memoria,&PID,sizeof(int),0);
    
    if(tam == sizeof(uint8_t))
    {
        recv(conexion_memoria,(uint8_t*)registro_datos,tam,MSG_WAITALL);
        log_info(logger,"PID: <%d> - Acción: <LEER> - Dirección Física: <%u> - Valor: <%hhu>",PID,direccion_fisica,*(uint8_t*)registro_datos);
    }
    else
    {
        recv(conexion_memoria,(uint32_t*)registro_datos,tam,MSG_WAITALL);
        log_info(logger,"PID: <%d> - Acción: <LEER> - Dirección Física: <%u> - Valor: <%u>",PID,direccion_fisica,*(uint32_t*)registro_datos);
    }
}

void mov_out(void* registro_direccion,void* registro_datos)
{   
    uint32_t direccion_fisica;
    if(tamanio(registro_direccion) == sizeof(uint8_t))direccion_fisica = traducir_direccion(*(uint8_t*)registro_direccion);
    else direccion_fisica = traducir_direccion(*(uint32_t*)registro_direccion);
    int comu = ESCRITURA;
    send(conexion_memoria,&comu,sizeof(int),0);
    send(conexion_memoria,&direccion_fisica,sizeof(int),0);
    int tam = tamanio(registro_datos);
    send(conexion_memoria,&tam,sizeof(int),0);
    send(conexion_memoria,&PID,sizeof(int),0);
    send (conexion_memoria,registro_datos,tam,0);

    recv(conexion_memoria,&comu,sizeof(int),MSG_WAITALL);

    if(tamanio(registro_direccion) == sizeof(uint8_t)) log_info(logger,"PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%u> - Valor: <%hhu>",PID,direccion_fisica,*(uint8_t*)registro_datos);
    else log_info(logger,"PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%u> - Valor: <%u>",PID,direccion_fisica,*(uint32_t*)registro_datos);
} 

void sum(void* destino,void* origen)
{
    int tamanio_destino = tamanio(destino);
    int tamanio_origen = tamanio(origen);
    if(tamanio_destino+ tamanio_origen == sizeof(uint32_t)*2)*(uint32_t*)destino += *(uint32_t*)origen;
    if(tamanio_destino + tamanio_origen == sizeof(uint8_t)*2)*(uint8_t*)destino += *(uint8_t*)origen;
    if(tamanio_destino == sizeof(uint32_t) && tamanio_origen == sizeof(uint8_t))*(uint32_t*)destino += *(uint8_t*)origen;
    if(tamanio_destino == sizeof(uint8_t) && tamanio_origen == sizeof(uint32_t))*(uint8_t*)destino += *(uint32_t*)origen;
}

void sub(void* destino,void* origen)
{
    int tamanio_destino = tamanio(destino);
    int tamanio_origen = tamanio(origen);
    if(tamanio_destino+ tamanio_origen == sizeof(uint32_t)*2)*(uint32_t*)destino -= *(uint32_t*)origen;
    if(tamanio_destino + tamanio_origen == sizeof(uint8_t)*2)*(uint8_t*)destino -= *(uint8_t*)origen;
    if(tamanio_destino == sizeof(uint32_t) && tamanio_origen == sizeof(uint8_t))*(uint32_t*)destino -= *(uint8_t*)origen;
    if(tamanio_destino == sizeof(uint8_t) && tamanio_origen == sizeof(uint32_t))*(uint8_t*)destino -= *(uint32_t*)origen;
}

void jnz(void* registro,uint32_t ins)
{

    if(tamanio(registro) == sizeof(uint32_t))if(*(uint32_t*)registro == 0)PC = ins;
    else if(*(uint8_t*)registro == 0)PC = ins;
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
    if(!tamanio)limpiar_tlb(PID);
}

void copy_string(void* tamanio)
{
    int diFisica = traducir_direccion(DI);
    int siFisica = traducir_direccion(SI);
    int comu = COPIAR;
    send(conexion_memoria,&comu,sizeof(int),0);
    send(conexion_memoria,&PID,sizeof(int),0);
    send(conexion_memoria,&diFisica,sizeof(int),0);
    send(conexion_memoria,&tamanio,sizeof(int),0);
    send(conexion_memoria,&siFisica,sizeof(int),0);

    recv(conexion_memoria,&comu,sizeof(int),MSG_WAITALL);       
}

void wait(char* recurso)
{
    sCall = ESPERAR;
    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    agregar_a_paquete(paquete,recurso,string_length(recurso)+1);
    free(recurso);
    sysCall = true;
}

void s1gnal(char* recurso)
{
    sCall = SENIAL;
    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    agregar_a_paquete(paquete,recurso,string_length(recurso)+1);
    free(recurso);
    sysCall = true;
}

void io_gen_sleep(char* interfaz,int unidad_trabajo)
{
    sCall = DORMIR;
    paquete = crear_paquete();
    nombre = interfaz;
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    agregar_a_paquete(paquete,nombre,string_length(nombre)+1);
    agregar_a_paquete(paquete,&unidad_trabajo,sizeof(int));
    free(interfaz);
    sysCall = true;
}

void io_stdin_read(char* interfaz,void* registro_direccion,void* registro_tamanio)
{
    sCall = IO_LEER;
    int direccionFisica;

    if(tamanio(registro_direccion) == sizeof(uint8_t))direccionFisica = traducir_direccion(*(uint8_t*)registro_direccion);
    else direccionFisica = traducir_direccion(*(uint32_t*)registro_direccion);

    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    nombre = interfaz;
    agregar_a_paquete(paquete,nombre,string_length(nombre));
    agregar_a_paquete(paquete,&direccionFisica,sizeof(int));
    agregar_a_paquete(paquete,(int*)registro_tamanio,tamanio(registro_tamanio));
    free(interfaz);
    sysCall = true;
}

void io_stdout_write(char* interfaz,void* registro_direccion,void* registro_tamanio)
{
    sCall = IO_ESCRIBIR;
    int direccionFisica;

    if(tamanio(registro_direccion) == sizeof(uint8_t))direccionFisica = traducir_direccion(*(uint8_t*)registro_direccion);
    else direccionFisica = traducir_direccion(*(uint32_t*)registro_direccion);

    paquete = crear_paquete();
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    nombre = interfaz;
    agregar_a_paquete(paquete,nombre,string_length(nombre));
    agregar_a_paquete(paquete,&direccionFisica,sizeof(int));
    agregar_a_paquete(paquete,(int*)registro_tamanio,tamanio(registro_tamanio));
    free(interfaz);
    sysCall = true;
}

void io_fs_create(char* interfaz,char* nombre_archivo)
{
    paquete = crear_paquete();
    sCall = CREAR;
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    agregar_a_paquete(paquete,interfaz,string_length(interfaz) +1);
    agregar_a_paquete(paquete,nombre_archivo,string_length(nombre_archivo)+1);
    free(interfaz);
    free(nombre_archivo);
    sysCall = true;
}   

void io_fs_delete(char* interfaz,char* nombre_archivo)
{
    paquete = crear_paquete();
    sCall = BORRAR;
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    agregar_a_paquete(paquete,interfaz,string_length(interfaz)+1);
    agregar_a_paquete(paquete,nombre_archivo,string_length(nombre_archivo)+1);
    free(interfaz);
    free(nombre_archivo);
    sysCall = true;
}

void io_fs_truncate(char* interfaz,char* nombre_archivo,void* registro_tamanio)
{
    paquete = crear_paquete();
    sCall = TRUNCAR;
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    agregar_a_paquete(paquete,interfaz,string_length(interfaz)+1);
    agregar_a_paquete(paquete,nombre_archivo,string_length(nombre_archivo)+1);
    agregar_a_paquete(paquete,(int*)registro_tamanio,tamanio(registro_tamanio));
    free(interfaz);
    free(nombre_archivo);
    sysCall = true;
}

void io_fs_write(char* interfaz,char* nombre_archivo,void* registro_direccion,void* registro_tamanio,void* registro_puntero_archivo)
{
    paquete = crear_paquete();
    sCall = FS_ESCRIBIR;
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    agregar_a_paquete(paquete,interfaz,string_length(interfaz)+1);
    agregar_a_paquete(paquete,nombre_archivo,string_length(nombre_archivo)+1);
    agregar_a_paquete(paquete,(int*)registro_direccion,tamanio(registro_direccion));
    agregar_a_paquete(paquete,(int*)registro_tamanio,tamanio(registro_tamanio));
    agregar_a_paquete(paquete,(int*)registro_puntero_archivo,tamanio(registro_puntero_archivo));
    free(interfaz);
    free(nombre_archivo);
    sysCall = true;
}

void io_fs_read(char* interfaz,char* nombre_archivo,void* registro_direccion,void* registro_tamanio,void* registro_puntero_archivo)
{
    paquete = crear_paquete();
    sCall = FS_LEER;
    agregar_a_paquete(paquete,&sCall,sizeof(int));
    agregar_a_paquete(paquete,interfaz,string_length(interfaz)+1);
    agregar_a_paquete(paquete,nombre_archivo,string_length(nombre_archivo)+1);
    agregar_a_paquete(paquete,(int*)registro_direccion,tamanio(registro_direccion));
    agregar_a_paquete(paquete,(int*)registro_tamanio,tamanio(registro_tamanio));
    agregar_a_paquete(paquete,(int*)registro_puntero_archivo,tamanio(registro_puntero_archivo));
    free(interfaz);
    free(nombre_archivo);
    sysCall = true;
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

uint32_t traducir_direccion(int direccion_logica)
{   
    int pagina = floor(direccion_logica/tam_pagina);

    int offset = direccion_logica%tam_pagina;
    int traduccion = obtener_marco(PID,pagina);

    if(traduccion == -1)
    {
        log_info(logger,"PID: <%d> - TLB MISS - Pagina: <%d>",PID,pagina);
        int comu = MARCO;

        send(conexion_memoria,&comu,sizeof(int),0);
        send(conexion_memoria,&PID,sizeof(int),0);
        send(conexion_memoria,&pagina,sizeof(int),0);

        recv(conexion_memoria,&traduccion,sizeof(int),MSG_WAITALL);

        log_info(logger,"PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>",PID,pagina,traduccion);
        aniadir_entrada_en_tlb(PID,pagina,traduccion);
    }
    else log_info(logger, "PID: <%d> - TLB HIT - Pagina: <%d>", PID,pagina);

    return traduccion*tam_pagina + offset;
}
