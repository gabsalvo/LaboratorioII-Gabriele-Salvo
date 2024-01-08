#ifndef BUFFER_H
#define BUFFER_H
#define PC_buffer_len 10

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void acquisisci_accesso_scrittura_buffer(pthread_mutex_t *mutexbuffer, int *dati, pthread_cond_t *full);
void rilascia_accesso_scrittura_buffer(pthread_mutex_t *mutexbuffer, int *dati, pthread_cond_t *empty);
void acquisisci_accesso_lettura_buffer(pthread_mutex_t *mutexbuffer, int *dati, pthread_cond_t *empty);
void rilascia_accesso_lettura_buffer(pthread_mutex_t *mutexbuffer, int *dati, pthread_cond_t *full);

#endif