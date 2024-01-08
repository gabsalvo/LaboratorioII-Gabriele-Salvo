#ifndef MANAGER_H
#define MANAGER_H
#define Num_elem 1000000
#define _POSIX_C_SOURCE 200809L


#include <pthread.h>
#include <search.h>
#include "buffer.h"
#include <fcntl.h>  
#include <unistd.h> 
#include <stdio.h>  
#include <stdlib.h>
#include <search.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
#include "writer.h"
#include "reader.h"
#include <signal.h>
#include <math.h>

typedef struct {
  pthread_t *caporeader;
  pthread_t *capowriter;
  pthread_mutex_t *mutexhashtable;
  pthread_cond_t *readgo;  
  pthread_cond_t *writego;
  int *waitingwriters;
  int *activereaders;
  int *activewriters;
  int *waitingreaders;
  int *hashtable;
  int *numeroentry;
  ENTRY **testa_lista_entry;
} manager;


// Dichiarazione delle funzioni
void stampa_numero_entry(int n, int fd);
void *gestorebody(void *argv);


#endif // MANAGER_H
