#include "writer.h"

int conta(char *s) {
  //S VA DEALLOCATA
  ENTRY *e = crea_entry(s, 1);
  ENTRY *r = hsearch(*e, FIND);

  if(r==NULL) { //non c'è nessuna chiave di nome s
    distruggi_entry(e);
    return 0;  
  }
  
  coppia *c = (coppia *) r->data;
  int d = c->valore;
  distruggi_entry(e);
  return d;
}

void distruggi_entry(ENTRY *e) {
  free(e->key);
  free(e->data);
  free(e);
}
void distruggi_lista(ENTRY *head) {
  if(head!=NULL) {
    distruggi_lista(((coppia *) head->data)->next); 
    distruggi_entry(head);//free(head->key); free(head->data); free(head);
  }
}

ENTRY *crea_entry(char *s, int n) {
  ENTRY *e = malloc(sizeof(ENTRY)); 
   if (e == NULL) {
    perror("Errore malloc");
    exit(EXIT_FAILURE);
  }
  e->key = strdup(s); // salva copia di char s
  e->data = malloc(sizeof(coppia));
  if (e->key == NULL || e->data == NULL) {
    perror("Errore malloc");
    if(e->key != NULL) free(e->key);
    free(e);
    exit(EXIT_FAILURE);
  }
  coppia *c = (coppia *)e->data;
  c->valore = n;
  c->next = NULL;
  return e;
}

void aggiungi(char *s, int *numeroentry, ENTRY ** testa_lista_entry) {
  ENTRY *e = crea_entry(s, 1);
  ENTRY *r = hsearch(*e, FIND);
  //static int numeroentry = 0;
  if (r == NULL) {          // la entry è nuova
    r = hsearch(*e, ENTER); // r: ENTRY *
    if (r == NULL) {
      fprintf(stderr, "Errore o tabella piena\n");
      distruggi_entry(e);
      exit(EXIT_FAILURE);
    }
    coppia *c = (coppia *) e->data;
    c->next = *(testa_lista_entry);
    *(testa_lista_entry) = e;
    *(numeroentry)+=1;
  
  } 
  else {
    assert(strcmp(e->key, r->key) == 0);
    coppia *c = (coppia *)r->data;
    c->valore += 1;
    distruggi_entry(e); // questa non la devo memorizzare
  }
}

void *capowritersbody(void *argv) {
  capowriter * a = (capowriter *) argv;
  char *sep =".,:; \n\r\t";
  int fd = open("caposc",O_RDONLY);
   if (fd < 0) {
    perror("Errore apertura named pipe scrittore");
    exit(EXIT_FAILURE);
  }
  int ppindex = 0;
  while(1) {
    int lunghezzadadecodificare;
    ssize_t e;
    e = read(fd,&lunghezzadadecodificare,sizeof(lunghezzadadecodificare)); //leggo quanti byte mi stanno per arrivare
    if(e==0) break;
    int lunghezza = ntohl(lunghezzadadecodificare);
    char *s= malloc(sizeof(char)*(lunghezza + 1)); 
    if (s == NULL) {
      perror("Errore malloc");
      exit(EXIT_FAILURE);
    }
    e = read(fd,s,lunghezza*sizeof(char)); //leggo tanti byte quanti me ne sono stati comunicati prima
    assert(e!=0);
    s[lunghezza] = '\0';
    //effettuo la tokenizzazione
    char *token;
    char * deallocami = s;
    while ((token = strtok_r(s, sep, &s))) { //esco quando token==NULL
      //token è la stringa delimitata. inserisco token dentro il buffer in mutua esclusione
      acquisisci_accesso_scrittura_buffer(a->mutexbufferw,a->datiwcapow,a->fullw);
      char *inseriscimi = strdup(token); 
      a->bufferw[(ppindex ++) %PC_buffer_len] = inseriscimi;
      rilascia_accesso_scrittura_buffer(a->mutexbufferw,a->datiwcapow,a->emptyw);
    }
    free(deallocami); 
  }
  //chiudo la pipe in lettura
   if (close(fd) < 0) {
    perror("Errore chiusura file descriptor");
    exit(EXIT_FAILURE);
  }
  //sono uscito dal while, quindi comunico numw volte che non ho più nessun valore da mettere nel buffer w capow
  for(int i = 0;i< a->numw ;i++) {
      acquisisci_accesso_scrittura_buffer(a->mutexbufferw,a->datiwcapow,a->fullw);
      a->bufferw[(ppindex ++) %PC_buffer_len] = NULL;
      rilascia_accesso_scrittura_buffer(a->mutexbufferw,a->datiwcapow,a->emptyw);
  }
  //ho comunicato a tutti gli scrittori che devono terminare, dunque attendo la loro terminazione
  for(int i = 0;i<a->numw;i++) {
    if (pthread_join(a->arrayscrittori[i], NULL) != 0) {
      perror("Errore pthread_join");
      exit(EXIT_FAILURE);
    }
  }
  return NULL;
}

void * writersbody(void *argv) {
  writer *a = (writer *) argv;

  //devo leggere in mutua esclusione dal bufferw-capow
  while(1) {
    char *s;
    acquisisci_accesso_lettura_buffer(a->mutexbufferw,a->datiwcapow,a->emptyw);
    s = a->bufferw[*(a->ccindex) % PC_buffer_len];
    *(a->ccindex) += 1;
    rilascia_accesso_lettura_buffer(a->mutexbufferw,a->datiwcapow,a->fullw);
    if(s==NULL) //il caposcrittore ha messo null sul buffer per segnalare di aver finito, quindi esco dal ciclo e valuterò come 
      break; 
    //ho letto il dato dal buffer in mutua esclusione dal buffer w-capow, lo inserisco nella hashtable

    if (pthread_mutex_lock(a->mutexhashtable) != 0) {
            perror("Errore nel bloccare il mutex");
            exit(EXIT_FAILURE);
    }
    *(a->waitingwriters)+=1;
    while(*(a->activewriters)>0 || *(a->activereaders)>0){
      if (pthread_cond_wait(a->writego, a->mutexhashtable) != 0) {
                perror("Errore in pthread_cond_wait");
                exit(EXIT_FAILURE);
      }
    }
    *(a->waitingwriters)-=1; 
    *(a->activewriters)+=1; 

    if (pthread_mutex_unlock(a->mutexhashtable) != 0) {
            perror("Errore nel sbloccare il mutex");
            exit(EXIT_FAILURE);
    }

    aggiungi(s,a->numeroentry,a->testa_lista_entry);
    
    if (pthread_mutex_lock(a->mutexhashtable) != 0) {
            perror("Errore nel bloccare il mutex");
            exit(EXIT_FAILURE);
    }
    *(a->activewriters)-=1;
    if(*(a->waitingreaders)>0) {
      if (pthread_cond_broadcast(a->readgo) != 0) {
                perror("Errore in pthread_cond_broadcast");
                exit(EXIT_FAILURE);
            }
    } else {
      if (pthread_cond_signal(a->writego) != 0) {
                perror("Errore in pthread_cond_signal");
                exit(EXIT_FAILURE);
      }
     }
     if (pthread_mutex_unlock(a->mutexhashtable) != 0) {
            perror("Errore nel sbloccare il mutex");
            exit(EXIT_FAILURE);
    }
    free(s); 
  }
  return NULL;
}