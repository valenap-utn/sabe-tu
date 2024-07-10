#include "cpu.h"

uint32_t PC;
int PID;
uint8_t AX;
uint8_t BX;
uint8_t CX;
uint8_t DX;
uint32_t EAX;
uint32_t EBX;
uint32_t ECX;
uint32_t EDX;
uint32_t SI;
uint32_t DI;

bool sysCall = false;

t_paquete* paquete;

t_list *tlb;

int conexion_memoria;
int cant_entradas_tlb;
char* algoritmo_reemplazo;

int main(int argc, char* argv[]) {
    
    config = config_create("cpu.config");
    logger = log_create(".log","CPU",1,LOG_LEVEL_INFO);

    cant_entradas_tlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");

    algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_TLB");


    tlb = list_create();


    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA");

    recv(conexion_memoria,&tam_pagina,sizeof(int),MSG_WAITALL);
    
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
        while(interrupcion && !sysCall)
        {
            char* instruccion = fetch(conexion_memoria);
            t_list * ins = decode(instruccion);
            log_info(logger,"PID: <%d> - Ejecutando: <%s>",PID,instruccion);
            execute(ins);
            free(instruccion);
            list_destroy(ins);
            PC++;
        }
        devolver_contexto(conexion_kernel);
        interrupcion = 1;
        sysCall = false;
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

    send(conexion,&sysCall,sizeof(bool),0);

    if(sysCall)
    {
        enviar_paquete(paquete,conexion);
        eliminar_paquete(paquete);
    }

}

char* fetch(int conexion)
{
    log_info(logger,"PID: <%d> - FETCH - Program Counter: <%u>",PID,PC);
	int i = INSTRUCCION;
    send(conexion,&i,sizeof(int),0);
    send(conexion,&PID,sizeof(int),0);
    send(conexion,&PC,sizeof(uint32_t),0);

    recv(conexion,&i,sizeof(int),MSG_WAITALL);
    char* instruccion = malloc(i*sizeof(char));
	
    recv(conexion,instruccion,sizeof(char)*i,MSG_WAITALL);
    if('E' != instruccion[0])sprintf(instruccion,"%.*s",i-1,instruccion);
    else sprintf(instruccion,"%.*s",i,instruccion);
	return instruccion;
}

t_list *decode(char* instruccion)
{   
    char* instruccion_separada = string_new();
	string_append(&instruccion_separada, instruccion);
	char* separador = " ";
	char **traduccion = string_split(instruccion_separada, separador);
	t_list *operandos= list_create();

	if (string_equals_ignore_case(traduccion[0], "SET"))
	{
        list_add(operandos,(void*)SET);
        list_add_strAReg1_strtoul2(operandos, traduccion[1], traduccion[2]);
	}
	if (string_equals_ignore_case(traduccion[0], "SUM")) 
	{
        list_add(operandos,(void*)SUM);
        list_add_strAReg_1y2(operandos, traduccion[1], traduccion[2]);
	}
	if(string_equals_ignore_case(traduccion[0], "SUB"))
	{
        list_add(operandos,(void*)SUB);
        list_add_strAReg_1y2(operandos, traduccion[1], traduccion[2]);
	}
	if (string_equals_ignore_case(traduccion[0], "MOV_IN"))
	{
        list_add(operandos,(void*)MOV_IN);
        list_add_strAReg_1y2(operandos, traduccion[1], traduccion[2]);
	}
	if(string_equals_ignore_case(traduccion[0], "MOV_OUT"))
	{
        list_add(operandos,(void*)MOV_OUT);
        list_add_strAReg_1y2(operandos, traduccion[1], traduccion[2]);
	}
	if(string_equals_ignore_case(traduccion[0], "JNZ"))
	{
        list_add(operandos,(void*)JNZ);
        list_add_strAReg1_strtoul2(operandos, traduccion[1], traduccion[2]);
	}
	if(string_equals_ignore_case(traduccion[0], "RESIZE"))
	{
        list_add(operandos,(void*)RESIZE);
        list_add(operandos,(void*)strtoul(traduccion[1],NULL,10));
	}
    if(string_equals_ignore_case(traduccion[0], "COPY_STRING"))
	{
        list_add(operandos,(void*)COPY_STRING);
        list_add(operandos,(void*)strtoul(traduccion[1],NULL,10));
	}
    if(string_equals_ignore_case(traduccion[0], "WAIT")) //
	{
		list_add(operandos,(void*)WAIT);
        char* nombre = string_new();
	    string_append(&nombre, traduccion[1]);
		list_add(operandos,(void*)nombre);
	}
    if(string_equals_ignore_case(traduccion[0], "SIGNAL")) //
	{
		list_add(operandos,(void*)SIGNAL);
	    char* nombre = string_new();
	    string_append(&nombre, traduccion[1]);
		list_add(operandos,(void*)nombre);
	}
    if(string_equals_ignore_case(traduccion[0], "IO_GEN_SLEEP")) //
	{
        list_add(operandos,(void*)IO_GEN_SLEEP);
        char* nombre = string_new();
	    string_append(&nombre, traduccion[1]);
		list_add(operandos,(void*)nombre);
		list_add(operandos,atoi(traduccion[2]));
	}
    if(string_equals_ignore_case(traduccion[0], "IO_STDIN_READ")) //
	{
        list_add(operandos,(void*)IO_STDIN_READ);
        list_add_trad1_strAReg2y3(operandos, traduccion[1], traduccion[2], traduccion[3]);
	}
    if(string_equals_ignore_case(traduccion[0], "IO_STDOUT_WRITE")) //
	{
        list_add(operandos,(void*)IO_STDOUT_WRITE);
        list_add_trad1_strAReg2y3(operandos, traduccion[1], traduccion[2], traduccion[3]);
	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_CREATE"))
	{
        list_add(operandos,(void*)IO_FS_CREATE);
        list_add_trad1y2(operandos, traduccion[1], traduccion[2]);
	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_DELETE"))
	{
        list_add(operandos,(void*)IO_FS_DELETE);
        list_add_trad1y2(operandos, traduccion[1], traduccion[2]);
	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_TRUNCATE"))
	{
        list_add(operandos,(void*)IO_FS_TRUNCATE);
        list_add_trad1y2(operandos, traduccion[1], traduccion[2]);
        list_add(operandos,stringAregistro(traduccion[3]));
	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_WRITE")) 
	{
        list_add(operandos,(void*)IO_FS_WRITE);
        list_add_trad1y2(operandos, traduccion[1], traduccion[2]);
        list_add_strAReg34y5(operandos, traduccion[3], traduccion[4], traduccion[5]);
	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_READ"))
	{
        list_add(operandos,(void*)IO_FS_READ);
        list_add_trad1y2(operandos, traduccion[1], traduccion[2]);
        list_add_strAReg34y5(operandos, traduccion[3], traduccion[4], traduccion[5]);
	}
    if(string_contains(traduccion[0], "EXIT"))
	{
        list_add(operandos,(void*)EXIT);
	}

    free(instruccion_separada);
    string_array_destroy(traduccion);
	return operandos;
}

//PROPUESTA



void list_add_strAReg1_strtoul2(t_list* operandos, char* traduccion1, char* traduccion2){
    list_add(operandos, stringAregistro(traduccion1));
	list_add(operandos,(void*)strtoul(traduccion2,NULL,10));
} 

void list_add_strAReg_1y2(t_list* operandos, char* traduccion1, char* traduccion2){
    list_add(operandos,stringAregistro(traduccion1));
	list_add(operandos, stringAregistro(traduccion2));
} 

void list_add_trad1_strAReg2y3(t_list* operandos, char* traduccion1, char* traduccion2, char* traduccion3){
    char* nombre = string_new();
	string_append(&nombre, traduccion1);
	list_add(operandos,(void*)nombre);
    list_add(operandos,stringAregistro(traduccion2));
    list_add(operandos,stringAregistro(traduccion3));
} 

void list_add_trad1y2(t_list* operandos, char* traduccion1, char* traduccion2){
    list_add(operandos,(void*)traduccion1);
	list_add(operandos,(void*)traduccion2);
} 

void list_add_strAReg34y5(t_list* operandos, char* traduccion3, char* traduccion4, char* traduccion5){
    list_add(operandos,stringAregistro(traduccion3));
    list_add(operandos,stringAregistro(traduccion4));
    list_add(operandos,stringAregistro(traduccion5));
} 

void *stringAregistro(char* registro)
{
	char *Sregistro = string_new();
	string_append(&Sregistro,registro);
	if(string_equals_ignore_case(Sregistro, "AX"))
	{
		return &AX;
	}
	if(string_equals_ignore_case(Sregistro, "BX"))
	{
		return &BX;
	}
	if(string_equals_ignore_case(Sregistro, "CX"))
	{
		return &CX;
	}
	if(string_equals_ignore_case(Sregistro, "DX"))
	{
		return &DX;
	}
    if(string_equals_ignore_case(Sregistro, "EAX"))
	{
		return &EAX;
	}
	if(string_equals_ignore_case(Sregistro, "EBX"))
	{
		return &EBX;
	}
	if(string_equals_ignore_case(Sregistro, "ECX"))
	{
		return &ECX;
	}
	if(string_equals_ignore_case(Sregistro, "EDX"))
	{
		return &EDX;
	}
    if(string_equals_ignore_case(Sregistro, "SI"))
	{
		return &SI;
	}
	if(string_equals_ignore_case(Sregistro, "DI"))
	{
		return &DI;
	}
    if(string_equals_ignore_case(Sregistro, "PC"))
	{
		return &PC;
	}
    return NULL;
}

void execute(t_list *instruccion)
{
    switch((int)list_remove(instruccion,0))
    {
        case SET:
            set(list_remove(instruccion,0),(uint32_t)list_remove(instruccion,1));
        break;
        case SUM:
            sum(list_remove(instruccion,0),list_remove(instruccion,1));
        break;
        case SUB:
            sub(list_remove(instruccion,0),list_remove(instruccion,1));
        break;
        case JNZ:
            jnz(list_remove(instruccion,0),(uint32_t)list_remove(instruccion,1));
        break;
        case MOV_OUT:
            mov_out(list_remove(instruccion,0),list_remove(instruccion,1));
        break;
        case MOV_IN:
            mov_in(list_remove(instruccion,0),list_remove(instruccion,1));
        break;
        case RESIZE:
            resize((int)list_remove(instruccion,0));
        break;
        case COPY_STRING:
            copy_string(list_remove(instruccion,0));
        break;
        case WAIT:
            wait(list_remove(instruccion,0));
        break;
        case SIGNAL:
            s1gnal(list_remove(instruccion,0));
        break;
        case IO_GEN_SLEEP:
            io_gen_sleep(list_remove(instruccion,0),list_remove(instruccion,1));
        break;
        case IO_STDIN_READ:
            io_stdin_read(list_remove(instruccion,0),list_remove(instruccion,1),list_remove(instruccion,2));
        break;
        case IO_STDOUT_WRITE:
            io_stdout_write(list_remove(instruccion,0),list_remove(instruccion,1),list_remove(instruccion,2));
        break;
        case IO_FS_CREATE:
            io_fs_create(list_remove(instruccion,0),list_remove(instruccion,1));
        break;
        case IO_FS_DELETE:
            io_fs_delete(list_remove(instruccion,0),list_remove(instruccion,1));
        break;
        case IO_FS_READ:
            io_fs_read(list_remove(instruccion,0),list_remove(instruccion,1),list_remove(instruccion,2),list_remove(instruccion,3),list_remove(instruccion,4));
        break;
        case IO_FS_WRITE:
            io_fs_write(list_remove(instruccion,0),list_remove(instruccion,1),list_remove(instruccion,2),list_remove(instruccion,3),list_remove(instruccion,4));
        break;
        case IO_FS_TRUNCATE:
            io_fs_truncate(list_remove(instruccion,0),list_remove(instruccion,1),list_remove(instruccion,2));
        break;
        case EXIT:
            salir();
        break;
        default:
            log_error(logger,"error, instruccion desconocida");
        break;
    }
}