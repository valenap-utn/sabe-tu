#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "utils.h"
#include<pthread.h>

int conexion_memoria;

pthread_t interrupciones;
pthread_t dispatch;

void interrupt();
void instrucciones();