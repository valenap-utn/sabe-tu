#include "TLB.h"


//devuelve el marco encontrado, si no encuentra la entrada la agrega a la TLB
//y devuelve el marco que se le haya asignado
int buscar_en_TLB(int pid, int pagina)
{
    for(int i = 0; i <= list_size(tlb); i++)
    {
        if(list_get(tlb,i)->pid == pid && list_get(tlb,i)->pagina == pagina)
        {
            log_info(logger, "PID: <%d> - TLB HIT - Pagina: <%d>", pid, pagina);
            //Actualizo ultimo uso (para LRU)
            if(strcmp(algoritmo_reemplazo, "LRU"))
            {
               TLB* entrada = list_remove(tlb,i);
               list_add(tlb,entrada);
            }
            return list_get(tlb,i)->marco;
        }
    }

    log_info(logger, "PID: <%d> = TLB MISS - Pagina: <%d>", pid, pagina);

    //Obtenemos el marco (desde la memoria) - funcion a desarrollar
    int marco = obtener_marco(pid, pagina);

    if(marco != -1)
    {
        aniadir_entrada_en_tlb(tlb, pid, pagina, marco);
    }

    log_info(logger, "PID: <%d> - OBTENER MARCO - Pagina: <%d> - Marco: <%d>", pid, pagina, marco);

    return marco;
}

void aniadir_entrada_en_tlb(int pid, int pagina, int marco)
{
    //Si no hay entradas libres en la TLB
    if(list_size(tlb) == cant_entradas_tlb)
    {
        free(list_remove(tlb,0));
    }
    //Actualizo la entrada de la TLB
    TLB* entrada = malloc(sizeof(TLB));
    entrada->pid = pid;
    entrada->pagina = pagina;
    entrada->marco = marco;
    list_add(tlb,entrada);
}

int  obtener_marco(int pid, int pagina)
{
    int i = MARCO;
    send(conexion_memoria,&i,sizeof(int),0);
    send(conexion_memoria,&pid,sizeof(int),0);
    send(conexion_memoria,&pagina,sizeof(int),0);

    recv(conexion_memoria,&i,sizeof(int),MSG_WAITALL);
    return i;
}