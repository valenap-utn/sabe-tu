#include "entradasalida.h"

int conexion_memoria;

char* bitmap_global;

t_bitarray *bits;

t_list* archivos;

char* archivo_bloques;

int block_size;
int block_count;

int conexion_kernel;

int main(int argc, char* argv[]){
    
    char *con = string_new();
    // string_append(&con, "entradasalida/");
    string_append(&con, argv[1]);
    string_append(&con,".config");
    config = config_create(con);
    char *log = string_new();
    // string_append(&log, "entradasalida/");
    string_append(&log,argv[1]);
    string_append(&log,".log");
    logger = log_create(log,argv[1],1,LOG_LEVEL_INFO);

    log_info(logger,"%s",config_get_string_value(config,"IP_MEMORIA"));
    log_info(logger,"%s",config_get_string_value(config,"PUERTO_MEMORIA"));

    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA");
    
    char* nombreBis = string_new();
    string_append(&nombreBis,argv[1]);
    string_append(&nombreBis,"\0");
    conexion_kernel = conectar("PUERTO_KERNEL","IP_KERNEL");
    int tipo = tipoInter(config_get_string_value(config,"TIPO_INTERFAZ"));
    int largoNombre = string_length(nombreBis);
    send(conexion_kernel,&tipo,sizeof(int),0);
    send(conexion_kernel,&largoNombre,sizeof(int),0);
    send(conexion_kernel,nombreBis,sizeof(char)*largoNombre,0);

    // se crea una interfaz
    if(tipo == DIALFS)interfs();
    inter(conexion_kernel);
    return 0;
}


void inter(int conexion_kernel)
{

    int peticion, tamanio,direccion,comunicacion,pid;
    char* output,*input;
    int ok = 1;
    while(1)
    {
        recv(conexion_kernel,&peticion,sizeof(int),MSG_WAITALL);
        switch(peticion)
        {
            case IO_GEN_SLEEP:
                int cantidad;
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&cantidad,sizeof(int),MSG_WAITALL);
                log_info(logger,"PID: <%d> - Operacion: <DORMIR>",pid);

                usleep(config_get_int_value(config,"TIEMPO_UNIDAD_TRABAJO")*cantidad*1000);

                send(conexion_kernel,&cantidad,sizeof(int),0);
            break;
            case IO_STDIN_READ:
               
                recv(conexion_kernel,&direccion,sizeof(int),MSG_WAITALL);
                 recv(conexion_kernel,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);

                log_info(logger,"PID: <%d> - Operacion: <LEER>",pid);
                input = readline(">");
                comunicacion = ESCRIBIR;
                send(conexion_memoria,&comunicacion,sizeof(int),0);
                
                
                send(conexion_memoria,&direccion,sizeof(int),0);
                send(conexion_memoria,&tamanio,sizeof(int),0);
                send(conexion_memoria,&pid,sizeof(int),0);
                send(conexion_memoria,input,tamanio,0);

                recv(conexion_memoria,&comunicacion,sizeof(int),MSG_WAITALL);

                send(conexion_kernel,&ok,sizeof(int),0);
            break;
            case IO_STDOUT_WRITE:
            
                recv(conexion_kernel,&direccion,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);
                
                log_info(logger,"PID: <%d> - Operacion: <ESCRIBIR>",pid);
                comunicacion = LEER;
                send(conexion_memoria,&comunicacion,sizeof(int),0);
                send(conexion_memoria,&direccion,sizeof(int),0);
                send(conexion_memoria,&tamanio,sizeof(int),0);
                send(conexion_memoria,&pid,sizeof(int),0);
                output = malloc(sizeof(char)*tamanio);
                recv(conexion_memoria,output,tamanio,MSG_WAITALL);
                sprintf(output,"%.*s",tamanio,output);
                log_info(logger,"%s",output);
                
                send(conexion_kernel,&ok,sizeof(int),0);
            break;

        }
    }
}


char nombre_archivo[20];

void interfs()
{
    block_size = config_get_int_value(config,"BLOCK_SIZE");
    block_count = config_get_int_value(config,"BLOCK_COUNT");
    char* path_bloques = string_new();
    string_append(&path_bloques,config_get_string_value(config,"PATH_BASE_DIALFS"));
    char* path_bitmap = string_duplicate(path_bloques);
    string_append(&path_bloques,"bloques.dat");
    string_append(&path_bitmap,"bitmap.dat");
    FILE *bloques = fopen(path_bloques,"rb");
    FILE *bitmap = fopen(path_bitmap,"rb");
    if(!bloques)inicializar_bloques(path_bloques);
    else fclose(bloques);
    if(!bitmap)inicializar_bitmap(path_bitmap);
    else fclose(bitmap);

    archivos = list_create();

    bitmap_global = malloc(block_count/8);
    bits = bitarray_create(bitmap_global,block_count);
    int bloques_libres = cargar_bitmap(path_bitmap);
    cargar_bloques(path_bloques);

    
    int peticion,tamanio,direccion,puntero,comunicacion, pid;
    t_config *metadata;
    char* path;
    archivo *archivo;
    int ok = 1;
    while(1)
    {
        recv(conexion_kernel,&peticion,sizeof(int),MSG_WAITALL);
        recv(conexion_kernel,&tamanio,sizeof(int),MSG_WAITALL);
        recv(conexion_kernel,nombre_archivo,tamanio,MSG_WAITALL);

        archivo = list_find(archivos,comparar_archivo);

        if(!archivo)
        {
            sumar_a_la_lista(nombre_archivo);
            archivo = list_find(archivos,comparar_archivo);
        }

        switch(peticion)
        {
            
            case IO_FS_CREATE:
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);
                

                archivo = malloc(sizeof(archivo));
                archivo->bloque_inicial = bloque_inicial();
                archivo->tamanio = 0;

                bitarray_set_bit(bits,archivo->bloque_inicial);


                char* nombre = string_new();
                string_append(&nombre,nombre_archivo);
                archivo->nombre = nombre;

                list_add(archivos,archivo);

                bloques_libres--;
                actualizar_metadata(archivo);

                log_info(logger, "PID: <%d> - Crear Archivo: <%s>", pid, nombre);          
                break;
            case IO_FS_DELETE:
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);

                
                list_remove_element(archivos,archivo);

                liberar_espacio(archivo->bloque_inicial,archivo->tamanio,0);
                
                bloques_libres += archivo->tamanio / block_size +1;
                free(archivo->nombre);
                free(archivo);

                path = string_new();
                string_append(&path,config_get_string_value(config,"PATH_BASE_DIALFS"));
                string_append(&path,nombre_archivo);
                remove(path);
                log_info(logger,"PID: <%d> - Eliminar Archivo: <%s>",pid,nombre_archivo);
            break;
            case IO_FS_TRUNCATE:
                recv(conexion_kernel,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);



                if(archivo->tamanio > tamanio)liberar_espacio(archivo->bloque_inicial,archivo->tamanio,tamanio);
                else asignar_espacio(archivo,tamanio,pid,bloques_libres);

                bloques_libres += (archivo->tamanio - tamanio)/block_size +1;

                archivo->tamanio = tamanio;


                actualizar_metadata(archivo);
                log_info(logger,"PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%d>",pid,nombre_archivo,tamanio);
            break;
            case IO_FS_READ:

                recv(conexion_kernel,&direccion,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&tamanio,sizeof(int),MSG_WAITALL);
                
                recv(conexion_kernel,&puntero,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);



                comunicacion = ESCRIBIR;
                send(conexion_memoria,&comunicacion,sizeof(int),0);
                
                send(conexion_memoria,&direccion,sizeof(int),0);
                send(conexion_memoria,&tamanio,sizeof(int),0);
                send(conexion_memoria,&pid,sizeof(int),0);
                send(conexion_memoria,&archivo_bloques[archivo->bloque_inicial*block_size + puntero],tamanio,0);
                log_info(logger,"PID: <%d> - Leer Archivo: <%s> - Tamaño a Leer: <%d> - Puntero Archivo: <%d>",pid,nombre_archivo,tamanio,puntero);
            break;
            case IO_FS_WRITE:

                recv(conexion_kernel,&direccion,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&tamanio,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&puntero,sizeof(int),MSG_WAITALL);
                recv(conexion_kernel,&pid,sizeof(int),MSG_WAITALL);



                comunicacion = LEER;
                send(conexion_memoria,&comunicacion,sizeof(int),0);
                send(conexion_memoria,&direccion,sizeof(int),0);
                send(conexion_memoria,&tamanio,sizeof(int),0);
                send(conexion_memoria,&pid,sizeof(int),0);

                recv(conexion_memoria,&archivo_bloques[archivo->bloque_inicial*block_size + puntero],tamanio,MSG_WAITALL);
                log_info(logger,"PID: <%d> - Escribir Archivo: <%s> - Tamaño a Escirbir: <%d> - Puntero Archivo: <%d>",pid,nombre_archivo,tamanio,puntero);
            break;
        }
        send(conexion_kernel,&ok,sizeof(int),0);
        actualizar_archivo_bloques();
        actualizar_archivo_bitmap();
    }
}


void sumar_a_la_lista(char *nombre_archivo)
{
    char* path = string_new();
    string_append(&path,config_get_string_value(config,"PATH_BASE_DIALFS"));
    string_append(&path,nombre_archivo);

    t_config *metadata = config_create(path);
    if(metadata)
    {
        archivo *a = malloc(sizeof(archivo));
        a->nombre = nombre_archivo;
        a->bloque_inicial = config_get_int_value(metadata,"BLOQUE_INICIAL");
        a->tamanio = config_get_double_value(metadata,"TAMANIO");

        list_add(archivos,a);
    }
}

bool comparar_archivo(void* archivo1)
{
    return string_equals_ignore_case(((archivo*)archivo1)->nombre,nombre_archivo);
}

int tipoInter(char* s)
{
    char *c = string_new();
    string_append(&c,s);
    if(string_equals_ignore_case(c,"Generica"))return GENERICA;
    if(string_equals_ignore_case(c,"STDIN"))return STDIN;
    if(string_equals_ignore_case(c,"STDOUT"))return STDOUT;
    if(string_equals_ignore_case(c,"DialFS"))return DIALFS;
    return -1;
}


void inicializar_bloques(char* path_bloques)
{
    FILE* bloques = fopen(path_bloques,"wb");
    int cero = 0;
    int tamanio = block_count*block_size;
    for(int i = 0;i < tamanio;i++)fwrite(&cero,sizeof(int),1,bloques);
    fclose(bloques);
}

void inicializar_bitmap(char* path_bitmap)
{
    FILE* bitmap = fopen(path_bitmap,"wb");
    int cero = 0;
    int tamanio = block_count/8;
    for(int i = 0;i < tamanio;i++)fwrite(&cero,sizeof(char),1,bitmap);
    fclose(bitmap);
}

int asignar_espacio(archivo *archivo,int tamanio,int pid,int bloques_libres)
{
    int contador = 0;
    int bloques = (int)ceil((double)tamanio/block_size);
    int i = 0;
    while(contador < bloques && i < block_count)
    {   
        if(bitarray_test_bit(bits,i))contador = 0;
        else contador++;
        i++;
    }
    if(contador < bloques)
    {
        if(bloques_libres > bloques)
        {
            compactacion(archivo,pid);
            return asignar_espacio(archivo,tamanio,pid,bloques_libres);
        }
    }
    else
    {
        sprintf(&archivo_bloques[(i - bloques)*block_size],"%.*s",archivo->tamanio,&archivo_bloques[archivo->bloque_inicial*block_size]);
        actualizar_bitmap(archivo->bloque_inicial,(int)ceil(archivo->tamanio/block_size),i - bloques,(int)ceil((double)tamanio/block_size));
        archivo->tamanio = tamanio;
        archivo->bloque_inicial = i -bloques;
        actualizar_metadata(archivo);
    }
    return i - bloques;
}

void actualizar_bitmap(int inicial_limpiar,int tamanio_limpiar,int inicial_settear,int tamanio_settear)
{
    if(!tamanio_limpiar)tamanio_limpiar++;
    for(int i = 0;i < tamanio_limpiar;i++)bitarray_clean_bit(bits,inicial_limpiar+i);
    for(int i = 0;i < tamanio_settear;i++)bitarray_set_bit(bits,inicial_settear+i);
}

void actualizar_metadata(archivo *arch)
{
        char* path = string_new();
        string_append(&path,config_get_string_value(config,"PATH_BASE_DIALFS"));
        string_append(&path,arch->nombre);
        
        FILE *f = fopen(path,"w");
        fclose(f);
        t_config *metadata = config_create(path);
        char* bi = malloc(10);
        sprintf(bi,"%d",arch->bloque_inicial);
        config_set_value(metadata,"BLOQUE_INICIAL",bi);
        sprintf(bi,"%f",arch->tamanio);
        config_set_value(metadata,"TAMANIO",bi);
        config_save(metadata);
        config_destroy(metadata);

        free(bi);
        free(path);
}

int cargar_bitmap(char* path)
{
    
    FILE* bitmap = fopen(path,"rb");
    int byte, bloques_libres = 0;
    int byteDescompuesto[8];
    fread(&byte,1,1,bitmap);


    for(int numeroByte = 0;numeroByte < block_count/8;numeroByte++)
    {
        descomponerByte(byte,byteDescompuesto);
        for(int numeroBit =0;numeroBit< 8;numeroBit++)
        {
            if(byteDescompuesto[numeroBit])bitarray_set_bit(bits,numeroByte+numeroBit);
            else 
            {
                bitarray_clean_bit(bits,numeroByte*8+numeroBit);
                bloques_libres++;
            }
        }
        fread(&byte,1,1,bitmap);
    }
    fclose(bitmap);
    return bloques_libres;
}

void descomponerByte(char byte,int* byteDescompuesto)
{
    for (int i = 7; i >= 0; i--) {
        byteDescompuesto[i] = (byte >> i) & 1;
    }
}

void liberar_espacio(int bloque_inicial,int tamanio_actual,int tamanio_final)
{
    int bloques_iniciales = tamanio_actual/block_size + 1;
    int bloques_finales = tamanio_final/block_size + 1;

    for(int i = 0;i < bloques_iniciales - bloques_finales; i++)
    {
        bitarray_clean_bit(bits,bloque_inicial+bloques_iniciales-i);
        vaciar_bloque(bloque_inicial+bloques_iniciales-i);
    }
}

void vaciar_bloque(int bloque)
{
    for(int i = 0; i < block_size;i++)archivo_bloques[bloque*block_size +i] = '\0';
}

void cargar_bloques(char* path)
{
    int tamanio = block_size*block_count;
    archivo_bloques = malloc(tamanio);
    FILE* bloques = fopen(path,"rb");
    fread(archivo_bloques,tamanio,1,bloques);
    fclose(bloques);
}

void actualizar_archivo_bloques()
{
    char* path_bloques = string_new();
    string_append(&path_bloques,config_get_string_value(config,"PATH_BASE_DIALFS"));
    string_append(&path_bloques,"bloques.dat");
    int tamanio = block_size*block_count;
    FILE* bloques = fopen(path_bloques,"wb");
    fwrite(archivo_bloques,tamanio,1,bloques);
    free(path_bloques);
    fclose(bloques);

}


void actualizar_archivo_bitmap()
{
    char* path_bitmap = string_new();
    string_append(&path_bitmap,config_get_string_value(config,"PATH_BASE_DIALFS"));
    string_append(&path_bitmap,"bitmap.dat");
    int tamanio = block_count/8;
    FILE* bitmap = fopen(path_bitmap,"wb");
    fwrite(bitmap_global,tamanio,1,bitmap);
    free(path_bitmap);
    fclose(bitmap);
}

void compactacion(archivo* archivo1,int pid)
{
    log_info(logger,"PID: <%d> - Inicio Compactación.",pid);
    list_remove_element(archivos,archivo1);
    char archivo_principal[(int)archivo1->tamanio /block_size];
    sprintf(archivo_principal,"%.*s",archivo1->tamanio,&archivo_bloques[archivo1->bloque_inicial*block_size]);
    list_sort(archivos,bloque_inicial_archivo);
    int nuevo_bloque_inicial =0;
    for(int i = 0;i < list_size(archivos);i++)
    {
        archivo *a_compactar = list_get(archivos,i);

        
        pasar_pagina(nuevo_bloque_inicial,a_compactar);
        
        a_compactar->bloque_inicial = nuevo_bloque_inicial;

        actualizar_metadata(a_compactar);
        nuevo_bloque_inicial += (int)ceil(a_compactar->tamanio/block_size);
    }

    archivo1->bloque_inicial = nuevo_bloque_inicial;
    sprintf(&archivo_bloques[nuevo_bloque_inicial*block_size],"%.*s",archivo1->tamanio,archivo_principal);
    int bloques_ocupados = nuevo_bloque_inicial + (int)ceil(archivo1->tamanio/block_size);
    compactar_bitmap(bloques_ocupados);

    for(int i =0;i < block_count - bloques_ocupados;i++)vaciar_bloque(bloques_ocupados + i + 1);

    usleep(config_get_int_value(config, "RETRASO_COMPACTACION") * 1000);
    
    log_info(logger,"PID: <%d> - Fin Compactación.",pid);
}

void compactar_bitmap(int bloques_ocupados)
{
    for(int i = 0; i < bloques_ocupados;i++)bitarray_set_bit(bits,i);
    for(int i = bloques_ocupados; i < block_count;i++)bitarray_clean_bit(bits,i);
}
bool bloque_inicial_archivo(void* archivo1, void* archivo2){
    return ((archivo*)archivo1)->bloque_inicial < ((archivo*)archivo2)->bloque_inicial;
}

int bloque_inicial()
{
    int i = 0;
    while(i < block_count && bitarray_test_bit(bits,i))i++;
    return i;
}

void pasar_pagina(int new_bloque_inicial,archivo *arch)
{
    int bloques = ceil(arch->tamanio / block_size);
    for(int i = 0;i < bloques*block_size;i++)
    {
        archivo_bloques[new_bloque_inicial*block_size + i] = archivo_bloques[arch->bloque_inicial*block_size + i];
    }
}