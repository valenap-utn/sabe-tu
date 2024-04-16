#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <commons/config.h>
#include <commons/collections/list.h>
#include <pcb.h>
#include<pthread.h>
#include<semaphore.h>


t_list *ready;
t_list *new;
t_list *blocked;

// PCB *execute;
void planFIFO(void);
#endif