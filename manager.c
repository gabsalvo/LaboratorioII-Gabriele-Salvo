#include "manager.h"

void stampa_numero_entry(int n, int fd) {
  ssize_t e=0;
  int i=0;
  if(n<10) {
    char buffer[] = "Numero entry: X\n";
    buffer[14] = '0'+n;
    e = write(fd,buffer,16);
  }
  else if(n<100) {
    char buffer[] = "Numero entry: XX\n";
    buffer[14] = '0'+ n/10;
    buffer[15] = '0' + n%10;
    e = write(fd,buffer,17);
  }
  else if(n<1000) {
    char buffer[] = "Numero entry: XXX\n";
    buffer[14] = '0'+ n/100;
  //caso 987
    int tmp = n%100; //87
    buffer[15] = '0' + tmp/10; // Cifra delle decine
    buffer[16] = '0' + n%10; // Cifra delle decine
    e = write(fd,buffer,18); 
  }
  else if(n<10000) {
    char buffer[] = "Numero entry: XXXX\n";
    while(i<4) {
      buffer[14+i] = '0'+ (n / (int) pow(10,3-i))%10;
      i++;
    }
    e = write(fd,buffer,19); 
  }
  else if(n<100000) {
    char buffer[] = "Numero entry: XXXXX\n";
      while(i<5) {
        buffer[14+i] = '0'+ (n / (int) pow(10,4-i))%10;
        i++;
      }
    e = write(fd,buffer,20); 
  }
  else if(n<1000000) {
    char buffer[] = "Numero entry: XXXXXX\n";
      while(i<6) {
        buffer[14+i] = '0'+ (n / (int) pow(10,5-i))%10;
        i++;
      }
    e = write(fd,buffer,21);
  }
   else if (n >= 1000000) {
    fprintf(stderr, "Numero entry registrate maggiore del numero di entry possibile\n");
    exit(EXIT_FAILURE);
  }

  if (e < 0) {
    perror("Errore write");
    exit(EXIT_FAILURE);
  }
 
}

void *gestorebody(void *argv) {
  manager *a = (manager *) argv;
  //mi metto in attesa di tutti i segnali
  sigset_t mask;
  sigfillset(&mask);
  int s;
  volatile sig_atomic_t continua = 1;
  while(continua==1) {
    int e = sigwait(&mask,&s);
    if(e!=0) {
      perror("Errore sigwait");
      exit(EXIT_FAILURE);
    }
    switch (s) {
      case SIGINT:
        //blocco tutti i segnali per evitare di andare in deadlock
        pthread_sigmask(SIG_BLOCK,&mask,NULL);
        //devo leggere la variabile globale in mutua esclusione perché sennò potrei leggerla mentre 
        //viene modificata da uno scrittore
        if (pthread_mutex_lock(a->mutexhashtable) != 0) {
        perror("Errore nel bloccare il mutex");
        exit(EXIT_FAILURE);
        }
        *(a->waitingreaders)+=1;
        while(*(a->activewriters)>0) {
          if (pthread_cond_wait(a->readgo, a->mutexhashtable) != 0) {
              perror("Errore in pthread_cond_wait");
              exit(EXIT_FAILURE);
          }
        }
        *(a->waitingreaders)-=1;
        *(a->activereaders)+=1;
        if (pthread_mutex_unlock(a->mutexhashtable) != 0) {
            perror("Errore nel sbloccare il mutex");
            exit(EXIT_FAILURE);
        }
        //stampo il numero di entry
        stampa_numero_entry(*(a->numeroentry),2);
        if (pthread_mutex_lock(a->mutexhashtable) != 0) {
            perror("Errore nel bloccare il mutex");
            exit(EXIT_FAILURE);
        }
        *(a->activereaders)-=1;
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
        //sblocco tutti i segnali per poterne ricevere altri
        pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
        break;
      case SIGTERM:
        //blocco tutti i segnali per evitare di essere interrotto
        pthread_sigmask(SIG_BLOCK,&mask,NULL);
        //attendo la terminazione dei capi
        if (pthread_join(*(a->caporeader), NULL) != 0) {
            perror("Errore pthread_join caporeader");
            exit(EXIT_FAILURE);
        }
        if (pthread_join(*(a->capowriter), NULL) != 0) {
            perror("Errore pthread_join capowriter");
            exit(EXIT_FAILURE);
        }
        //stampo su stdout il numero totale di stringhe contenute dentro la tabella hash
        //non ho bisogno di mutua esclusione perché hanno già terminato anche tutti gli scrittori e tutti i lettori
        stampa_numero_entry(*(a->numeroentry),1);
        distruggi_lista(*(a->testa_lista_entry)); 
        hdestroy();
        continua = 0;
        break;
      case SIGUSR1:
        // Blocco tutti i segnali per evitare di andare in deadlock
        pthread_sigmask(SIG_BLOCK, &mask, NULL);

        if (pthread_mutex_lock(a->mutexhashtable) != 0) {
            perror("Errore nel bloccare il mutex");
            exit(EXIT_FAILURE);
        }

        *(a->waitingwriters) += 1;
        while (*(a->activewriters) > 0 || *(a->activereaders) > 0) {
          if (pthread_cond_wait(a->writego, a->mutexhashtable) != 0) {
              perror("Errore in pthread_cond_wait");
              exit(EXIT_FAILURE);
          }
        }
        *(a->waitingwriters) -= 1;
        *(a->activewriters) += 1;

        // Dealloco tutto
        distruggi_lista(*(a->testa_lista_entry));
        hdestroy();
        *(a->numeroentry) = 0;
        *(a->testa_lista_entry) = NULL; 

        if (!hcreate(Num_elem)) {
            perror("Errore hcreate");
            exit(EXIT_FAILURE);
        }

        *(a->activewriters) -= 1;
        if (*(a->waitingreaders) > 0) {
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

        // Sblocco tutti i segnali per riceverne altri
        pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
        break;
    }
  }
  return NULL;
}
