#include "TLB.h"


//devuelve el marco encontrado, si no encuentra la entrada la agrega a la TLB
//y devuelve el marco que se le haya asignado
int buscar_en_TLB(int pid, int pagina)
{
    for(int i = 0; i <= list_size(tlb); i++)
    {
        if(((TLB*)list_get(tlb,i))->pid == pid && ((TLB*)list_get(tlb,i))->pagina == pagina)
        {
            log_info(logger, "PID: <%d> - TLB HIT - Pagina: <%d>", pid, pagina);
            //Actualizo ultimo uso (para LRU)
            if(strcmp(algoritmo_reemplazo, "LRU"))
            {
               TLB* entrada = list_remove(tlb,i);
               list_add(tlb,entrada);
            }
            return ((TLB*)list_get(tlb,i))->marco;
        }
    }

    log_info(logger, "PID: <%d> = TLB MISS - Pagina: <%d>", pid, pagina);

    //Obtenemos el marco (desde la memoria) - funcion a desarrollar
    int marco = obtener_marco(pid, pagina);

    if(marco != -1)
    {
        aniadir_entrada_en_tlb(pid, pagina, marco);
    }

    log_info(logger, "PID: <%d> - OBTENER MARCO - Pagina: <%d> - Marco: <%d>", pid, pagina, marco);

    return marco;
}

void aniadir_entrada_en_tlb(int pid, int pagina, int marco)
{
    //Si no hay entradas libres en la TLB
    if(list_size(tlb) == cant_entradas_tlb && cant_entradas_tlb != 0)
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

    bool encontrar_entrada(void* entrada)
    {
        return (((TLB*)entrada)->pid == pid && ((TLB*)entrada)->pagina == pagina);
    }
    TLB* entrada = list_find(tlb,encontrar_entrada);
    if(entrada == NULL) return -1;
	if(strcmp(config_get_string_value(config,"ALGORITMO_TLB"),"LRU") == 0){
	list_remove_element(tlb,entrada);
	list_add(tlb,entrada);
	}
     return entrada->marco;
}

void limpiar_tlb(int PID)
{
    bool son_del_proceso(void* entrada)
    {
        return ((TLB*)entrada)->pid == PID;
    }
    list_remove_and_destroy_by_condition(tlb,son_del_proceso,free);
}
