#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <search.h>
#include <arpa/inet.h>
#include <math.h>
#include <signal.h>
#include "writer.h"
#include "buffer.h"
#include "reader.h"
#include "manager.h"


int main(int argc, char **argv) {
  if (argc != 3) {
      fprintf(stderr, "Uso: %s <numero di scrittori> <numero di lettori>\n", argv[0]);
      exit(EXIT_FAILURE);
  }
  int numw = atoi(argv[1]);
  int numr = atoi(argv[2]);

  if(numw<=0 || numr<=0) {
      fprintf(stderr, "I numer di thread lettori e di thread scrittori devono essere entrambi strettamente positivi\n");
      exit(EXIT_FAILURE);
  }

  int hashtable = hcreate(Num_elem);
  if (hashtable == 0) {
      fprintf(stderr, "Errore nella creazione della hash Table\n");
      exit(EXIT_FAILURE);
  }

  //inizializzo il contatore del numero di ENTRY e la rispettiva lista
  ENTRY *testa_lista_entry = NULL;
  int numeroentry = 0;
  // inizializzo i mutex e le cv
  // ho bisogno di mutex per bufferw-cw, bufferr-cr,bufferrw
  pthread_mutex_t mutexbufferr = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t mutexhashtable = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t mutexbufferw = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t mutexfile = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t readgo = PTHREAD_COND_INITIALIZER; 
  pthread_cond_t writego = PTHREAD_COND_INITIALIZER;  
  pthread_cond_t emptyr = PTHREAD_COND_INITIALIZER;
  pthread_cond_t fullr = PTHREAD_COND_INITIALIZER;
  pthread_cond_t emptyw = PTHREAD_COND_INITIALIZER;
  pthread_cond_t fullw = PTHREAD_COND_INITIALIZER;
  char *bufferw[PC_buffer_len]; // buffer tra scrittori e capo è array di stringhe
  char *bufferr[PC_buffer_len]; // buffer tra lettori e capo è array di stringhe
  int datirw = 0;
  int datiwcapow = 0;
  int datircapor = 0;
  int waitingwriters = 0;
  int activereaders = 0;
  int activewriters = 0;
  int waitingreaders = 0;

  
  // blocco tutti i segnali
  sigset_t mask;
  sigfillset(&mask); //riempo la maschera di tutti i segnali
  pthread_sigmask(SIG_BLOCK,&mask,NULL); // blocco tutto 
  // faccio partire il thread gestore
  pthread_t gestore;
  pthread_t capolettore;
  pthread_t caposcrittore;
  pthread_t readers[numr];
  pthread_t writers[numw];
  manager datig;
  datig.caporeader = &capolettore;
  datig.capowriter = &caposcrittore;
  datig.mutexhashtable = &mutexhashtable;
  datig.readgo = &readgo;
  datig.writego = &writego;
  datig.activereaders = &activereaders;
  datig.activewriters = &activewriters;
  datig.waitingreaders = &waitingreaders;
  datig.waitingwriters = &waitingwriters;
  datig.hashtable = &hashtable;
  datig.numeroentry = &numeroentry;
  datig.testa_lista_entry = &testa_lista_entry;
  if (pthread_create(&gestore, NULL, &gestorebody, &datig) != 0) {
        perror("Errore nella creazione del thread gestore");
        exit(EXIT_FAILURE);
  }
  // faccio partire il thread capo lettore
  FILE *flettori = fopen("lettori.log", "w");
  if(flettori==NULL) {
      perror("Errore apertura lettori.log in scrittura");
      exit(EXIT_FAILURE);
  }
  caporeader dr;
  dr.mutexbufferr = &mutexbufferr;
  dr.bufferr = bufferr;
  dr.emptyr = &emptyr;
  dr.fullr = &fullr;
  dr.datircapor = &datircapor;
  dr.numr = numr;
  dr.arraylettori = readers;
  dr.f = flettori;
  if (pthread_create(&capolettore, NULL, &caporeaderbody, &dr) != 0) {
        perror("Errore nella creazione del thread capo lettore");
        exit(EXIT_FAILURE);
  }
  // faccio partire i lettori
  reader datareader[numr];
  int cindexr = 0;
  
  for (int i = 0; i < numr; i++) {
    datareader[i].mutexbufferr = &mutexbufferr; 
    datareader[i].ccindex = &cindexr; 
    datareader[i].bufferr = bufferr; 
    datareader[i].mutexhashtable = &mutexhashtable; 
    datareader[i].mutexfile = &mutexfile; 
    datareader[i].emptyr = &emptyr;
    datareader[i].readgo = &readgo;
    datareader[i].writego = &writego;
    datareader[i].fullr = &fullr;
    datareader[i].datircapor = &datircapor;
    datareader[i].datirw = &datirw;
    datareader[i].activereaders = &activereaders;
    datareader[i].activewriters = &activewriters;
    datareader[i].waitingreaders = &waitingreaders;
    datareader[i].waitingwriters = &waitingwriters;
    datareader[i].f = flettori;
    
    if (pthread_create(&readers[i], NULL, &readersbody, &datareader[i]) != 0) {
        perror("Errore nella creazione del thread lettore");
        exit(EXIT_FAILURE);
    }
  }
  
  //faccio partire il capo scrittore
  capowriter dw;
  dw.mutexbufferw = &mutexbufferw;
  dw.bufferw = bufferw;
  dw.emptyw = &emptyw;
  dw.fullw = &fullw;
  dw.datiwcapow = &datiwcapow;
  dw.numw = numw;
  dw.arrayscrittori = writers;
 
  if (pthread_create(&caposcrittore, NULL, &capowritersbody, &dw) != 0) {
        perror("Errore nella creazione del thread capo scrittore");
        exit(EXIT_FAILURE);
  }

  
  // faccio partire gli scrittori
  writer datawriter[numw];
  int cindexw = 0;
  for (int i = 0; i < numw; i++) {
    datawriter[i].mutexbufferw = &mutexbufferw; 
    datawriter[i].ccindex = &cindexw;
    datawriter[i].bufferw = bufferw;
    datawriter[i].mutexhashtable = &mutexhashtable;
    datawriter[i].readgo = &readgo;
    datawriter[i].writego = &writego;
    datawriter[i].emptyw = &emptyw;
    datawriter[i].fullw = &fullw;
    datawriter[i].datirw = &datirw;
    datawriter[i].datiwcapow = &datiwcapow;
    datawriter[i].activereaders = &activereaders;
    datawriter[i].activewriters = &activewriters;
    datawriter[i].waitingreaders = &waitingreaders;
    datawriter[i].waitingwriters = &waitingwriters;
    datawriter[i].numeroentry = &numeroentry;
    datawriter[i].testa_lista_entry = &testa_lista_entry;
    if (pthread_create(&writers[i], NULL, &writersbody, &datawriter[i]) != 0) {
            perror("Errore nella creazione del thread scrittore");
            exit(EXIT_FAILURE);
    }
  }
  
  if (pthread_join(gestore, NULL) != 0) {
        perror("Errore nel join del thread gestore");
        exit(EXIT_FAILURE);
  }

  return 0;
}
