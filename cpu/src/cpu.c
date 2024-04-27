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

bool sysCall = true;

t_paquete* paquete;


int main(int argc, char* argv[]) {
    
    config = config_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/cpu/cpu.config");
    logger = log_create("/home/utnso/tp-2024-1c-Grupo-Buenisimo/cpu/cpu.log","CPU",1,LOG_LEVEL_INFO);

    conexion_memoria = conectar("PUERTO_MEMORIA","IP_MEMORIA");
    
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
        while(interrupcion && sysCall)
        {
            char* instruccion = fetch(conexion_memoria);
            t_list * ins = decode(instruccion);
            execute(ins);
        }
        devolver_contexto(conexion_kernel);
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
    bool paq;
    if(paquete != NULL)
    {
        paq = true;
        send(conexion,&paq,sizeof(bool),0);
        enviar_paquete(paquete,conexion);
        eliminar_paquete(paquete);
        paquete = NULL;
    }
    else{
        paq = false;
        send(conexion,&paq,sizeof(bool),0);
    }
}

char* fetch(int conexion)
{
	int i = INSTRUCCION;
    send(conexion,&i,sizeof(int),0);
    send(conexion,&PID,sizeof(int),0);
    send(conexion,&PC,sizeof(int),0);

    recv(conexion,&i,sizeof(int),MSG_WAITALL);
    char* instruccion = malloc(i*sizeof(char));
	recv(conexion,instruccion,sizeof(char)*i,MSG_WAITALL);
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
		list_add(operandos, stringAregistro(traduccion[1]));
		list_add(operandos,(void*)strtoul(traduccion[2],NULL,10));
	}
	if (string_equals_ignore_case(traduccion[0], "SUM")) 
	{
        list_add(operandos,(void*)SUM);
		list_add(operandos,stringAregistro(traduccion[1]));
		list_add(operandos, stringAregistro(traduccion[2]));
	}
	if(string_equals_ignore_case(traduccion[0], "SUB"))
	{
        list_add(operandos,(void*)SUB);
		list_add(operandos,stringAregistro(traduccion[1]));
		list_add(operandos, stringAregistro(traduccion[2]));
	}
	if (string_equals_ignore_case(traduccion[0], "MOV_IN"))
	{
        list_add(operandos,(void*)MOV_IN);
		list_add(operandos,stringAregistro(traduccion[1]));
		list_add(operandos, stringAregistro(traduccion[2]));
	}
	if(string_equals_ignore_case(traduccion[0], "MOV_OUT"))
	{
        list_add(operandos,(void*)MOV_OUT);
		list_add(operandos,stringAregistro(traduccion[1]));
		list_add(operandos, stringAregistro(traduccion[2]));
	}
	if(string_equals_ignore_case(traduccion[0], "JNZ"))
	{
        list_add(operandos,(void*)JNZ);
		list_add(operandos,stringAregistro(traduccion[1]));
		list_add(operandos,(void*)strtoul(traduccion[2],NULL,10));
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
    if(string_equals_ignore_case(traduccion[0], "WAIT"))
	{
		list_add(operandos,(void*)WAIT);
		list_add(operandos,traduccion[1]);
	}
    if(string_equals_ignore_case(traduccion[0], "SIGNAL"))
	{
		list_add(operandos,(void*)SIGNAL);
		list_add(operandos,(void*)traduccion[1]);
	}
    if(string_equals_ignore_case(traduccion[0], "IO_GEN_SLEEP"))
	{
        list_add(operandos,(void*)IO_GEN_SLEEP);
		list_add(operandos,(void*)traduccion[1]);
		list_add(operandos,atoi(traduccion[2]));
	}
    if(string_equals_ignore_case(traduccion[0], "IO_STDIN_READ"))
	{
        list_add(operandos,(void*)IO_STDIN_READ);
		list_add(operandos,(void*)traduccion[1]);
		list_add(operandos,stringAregistro(traduccion[2]));
        list_add(operandos,stringAregistro(traduccion[3]));
	}
    if(string_equals_ignore_case(traduccion[0], "IO_STDOUT_WRITE"))
	{
        list_add(operandos,(void*)IO_STDOUT_WRITE);
		list_add(operandos,(void*)traduccion[1]);
		list_add(operandos,stringAregistro(traduccion[2]));
        list_add(operandos,stringAregistro(traduccion[3]));
	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_CREATE"))
	{
        list_add(operandos,(void*)IO_FS_CREATE);
		list_add(operandos,(void*)traduccion[1]);
		list_add(operandos,(void*)traduccion[2]);
	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_DELETE"))
	{
        list_add(operandos,(void*)IO_FS_DELETE);
		list_add(operandos,(void*)traduccion[1]);
		list_add(operandos,(void*)traduccion[2]);
	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_TRUNCATE"))
	{
        list_add(operandos,(void*)IO_FS_TRUNCATE);
		list_add(operandos,(void*)traduccion[1]);
		list_add(operandos,(void*)traduccion[2]);
        list_add(operandos,stringAregistro(traduccion[3]));
	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_WRITE"))
	{
        list_add(operandos,(void*)IO_FS_WRITE);
		list_add(operandos,(void*)traduccion[1]);
		list_add(operandos,(void*)traduccion[2]);
        list_add(operandos,stringAregistro(traduccion[3]));
        list_add(operandos,stringAregistro(traduccion[4]));
        list_add(operandos,stringAregistro(traduccion[5]));

	}
    if(string_equals_ignore_case(traduccion[0], "IO_FS_READ"))
	{
        list_add(operandos,(void*)IO_FS_READ);
		list_add(operandos,(void*)traduccion[1]);
		list_add(operandos,(void*)traduccion[2]);
        list_add(operandos,stringAregistro(traduccion[3]));
        list_add(operandos,stringAregistro(traduccion[4]));
        list_add(operandos,stringAregistro(traduccion[5]));
	}
    if(string_equals_ignore_case(traduccion[0], "EXIT"))
	{
        list_add(operandos,(void*)EXIT);
	}
    free(instruccion_separada);
    string_array_destroy(traduccion);
	return operandos;
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
    return NULL;
}

void execute(t_list *instruccion)
{
    switch((int)list_remove(instruccion,0))
    {
        case SET:

        break;
        case SUM:

        break;
        case SUB:

        break;
        case JNZ:

        break;
        case MOV_OUT:

        break;
        case MOV_IN:

        break;
        case RESIZE:

        break;
        case COPY_STRING:

        break;
        case WAIT:

        break;
        case SIGNAL:

        break;
        case IO_GEN_SLEEP:

        break;
        case IO_STDIN_READ:

        break;
        case IO_STDOUT_WRITE:

        break;
        case IO_FS_CREATE:

        break;
        case IO_FS_DELETE:

        break;
        case IO_FS_READ:

        break;
        case IO_FS_WRITE:

        break;
        case IO_FS_TRUNCATE:

        break;
        case EXIT:

        break;
        default:
            log_error(logger,"error, instruccion desconocida");
        break;
    }
}