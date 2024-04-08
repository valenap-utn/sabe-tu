#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include <commons/config.h>
#include <pthread.h>



pthread_t cpu;
pthread_t io;
pthread_t kernel;