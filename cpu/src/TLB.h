#ifndef TLB_H
#define TLB_H

#include"utils.h"

enum comunicacion_con_memoria
{
    INSTRUCCION,
    MARCO,
    LECTURA,
    ESCRITURA,
    AJUSTE
};

struct t_TLBEntry //entrada en TLB
{
    int pid;
    int pagina;
    int marco;
    int ultimo_uso; //para LRU
};

typedef struct t_TLBEntry TLB;

extern t_list *tlb;

extern int cant_entradas_tlb;
extern char* algoritmo_reemplazo;

extern int conexion_memoria;

int buscar_en_TLB(int pid, int pagina);
void aniadir_entrada_en_tlb(int pid, int pagina, int marco);
int  obtener_marco(int pid,int pagina);


#endif