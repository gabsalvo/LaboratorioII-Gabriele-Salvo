#ifndef READER_H
#define READER_H

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

typedef struct {
  int *ccindex;
  pthread_mutex_t *mutexhashtable;
  pthread_mutex_t *mutexbufferr;
  pthread_mutex_t *mutexfile;
  char **bufferr;
  pthread_cond_t *readgo;  
  pthread_cond_t *writego;   
  pthread_cond_t *emptyr; 
  pthread_cond_t *fullr;  
  int *datirw;
  int *datircapor;
  int *waitingwriters;
  int *activereaders;
  int *activewriters;
  int *waitingreaders;
  FILE *f;
} reader;

typedef struct {
  pthread_mutex_t *mutexbufferr;
  char **bufferr;
  pthread_cond_t *emptyr; 
  pthread_cond_t *fullr;  
  int *datircapor;
  int numr;
  pthread_t * arraylettori;
  FILE *f;
} caporeader;



// Dichiarazione delle funzioni
void * caporeaderbody(void *argv);
void *readersbody(void *argv);

#endif // READER_H
