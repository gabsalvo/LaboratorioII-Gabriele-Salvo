#ifndef WRITER_H
#define WRITER_H

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


typedef struct {
  int valore; // numero occorrenze della stringa
  ENTRY *next;
} coppia;


typedef struct {
  int *ccindex;
  pthread_mutex_t *mutexhashtable;
  pthread_mutex_t *mutexbufferw;
  char **bufferw;
  pthread_cond_t *readgo; 
  pthread_cond_t *writego;  
  pthread_cond_t *emptyw;
  pthread_cond_t *fullw;
  int *datiwcapow;
  int *datirw;
  int *waitingwriters;
  int *activereaders;
  int *activewriters;
  int *waitingreaders;
  int *numeroentry;
  ENTRY **testa_lista_entry;
} writer;

typedef struct {
  pthread_mutex_t *mutexbufferw;
  pthread_cond_t *emptyw; 
  pthread_cond_t *fullw; 
  char **bufferw;
  int *datiwcapow;
  int numw;
  pthread_t * arrayscrittori;
} capowriter;

// Dichiarazione delle funzioni
int conta(char *s);
void *capowritersbody(void *argv);
void *writersbody(void *argv);
void distruggi_entry(ENTRY *e);
void distruggi_lista(ENTRY *head);
ENTRY *crea_entry(char *s, int n);
void aggiungi(char *s, int *numeroentry, ENTRY **testa_lista_entry);

#endif // WRITER_H
