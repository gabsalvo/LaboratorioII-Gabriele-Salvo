#include "reader.h"

void * caporeaderbody(void *argv) {
  caporeader * a = (caporeader *) argv;
  //apro la pipe in lettura
  int fd = open("capolet",O_RDONLY);
  if(fd<0) {
    perror("Errore apertura named pipe lettore");
    exit(EXIT_FAILURE);
  }
  int ppindex = 0;
  char *sep =".,:; \n\r\t";

  while(1) {
    int lunghezzadadecodificare;
    ssize_t e;
    e = read(fd,&lunghezzadadecodificare,sizeof(lunghezzadadecodificare)); //leggo quanti byte mi stanno per arrivare
    if(e==0) break;
    int lunghezza = ntohl(lunghezzadadecodificare);
    char *s = malloc(sizeof(char)*(lunghezza+1)); 
    if(s==NULL) {
      perror("Errore malloc");
      exit(EXIT_FAILURE);
    }
    e = read(fd,s,lunghezza*sizeof(char)); //leggo tanti byte quanti me ne sono stati detti poc'anzi
    assert(e!=0);
    s[lunghezza] = '\0';
    
    char *deallocami = s;
    char *token;
    while ((token = strtok_r(s, sep, &s))) {
      acquisisci_accesso_scrittura_buffer(a->mutexbufferr,a->datircapor,a->fullr);
      char *inseriscimi = strdup(token); 
      a->bufferr[(ppindex++) %PC_buffer_len] = inseriscimi;
      rilascia_accesso_scrittura_buffer(a->mutexbufferr,a->datircapor,a->emptyr);
    }
    free(deallocami);
  }
  //chiudo la pipe in lettura
   if (close(fd) < 0) {
        perror("Errore chiusura file descriptor");
        exit(EXIT_FAILURE);
  }
  //sono uscito dal while, quindi comunico numr volte che non ho più nessun valore da mettere nel buffer r capor
  for(int i = 0;i< a->numr;i++) {
      acquisisci_accesso_scrittura_buffer(a->mutexbufferr,a->datircapor,a->fullr);
      a->bufferr[(ppindex++) %PC_buffer_len] =NULL;
      rilascia_accesso_scrittura_buffer(a->mutexbufferr,a->datircapor,a->emptyr);
  }
  //ho comunicato a tutti i lettori che devono terminare, dunque attendo la loro terminazione
  for(int i = 0;i< a->numr ;i++) {
    if (pthread_join(a->arraylettori[i], NULL) != 0) {
            perror("Errore pthread_join");
            exit(EXIT_FAILURE);
    }
  }
  
  //chiudo lettori.log
  if(fclose(a->f)!=0) {
     perror("Errore chiusura file");
     exit(EXIT_FAILURE);
  }
  
  return NULL;
}

void *readersbody(void *argv) {
    reader *a = (reader *) argv;

    while (1) {
        char *s;
        acquisisci_accesso_lettura_buffer(a->mutexbufferr, a->datircapor, a->emptyr);
        s = a->bufferr[*(a->ccindex) % PC_buffer_len];
        *(a->ccindex) += 1;
        rilascia_accesso_lettura_buffer(a->mutexbufferr, a->datircapor, a->fullr);

        if (s == NULL)
            break;

        if (pthread_mutex_lock(a->mutexhashtable) != 0) {
            perror("Errore nel bloccare il mutex");
            exit(EXIT_FAILURE);
        }
        *(a->waitingreaders) += 1;
        while (*(a->activewriters) > 0) {
            if (pthread_cond_wait(a->readgo, a->mutexhashtable) != 0) {
                perror("Errore in pthread_cond_wait");
                exit(EXIT_FAILURE);
            }
        }
        *(a->waitingreaders) -= 1;
        *(a->activereaders) += 1;
        if (pthread_mutex_unlock(a->mutexhashtable) != 0) {
            perror("Errore nel sbloccare il mutex");
            exit(EXIT_FAILURE);
        }

        int n = conta(s);

        if (pthread_mutex_lock(a->mutexhashtable) != 0) {
            perror("Errore nel bloccare il mutex");
            exit(EXIT_FAILURE);
        }
        *(a->activereaders) -= 1;
        if (*(a->activereaders) == 0 && *(a->waitingwriters) > 0) {
            if (pthread_cond_signal(a->writego) != 0) {
                perror("Errore in pthread_cond_signal");
                exit(EXIT_FAILURE);
            }
        }
        if (pthread_mutex_unlock(a->mutexhashtable) != 0) {
            perror("Errore nel sbloccare il mutex");
            exit(EXIT_FAILURE);
        }

        if (pthread_mutex_lock(a->mutexfile) != 0) {
            perror("Errore nel bloccare il mutex");
            exit(EXIT_FAILURE);
        }
        fprintf(a->f, "%s %d\n", s, n);
        if (pthread_mutex_unlock(a->mutexfile) != 0) {
            perror("Errore nel sbloccare il mutex");
            exit(EXIT_FAILURE);
        }

        free(s);
    }
    return NULL;
}

