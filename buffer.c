#include "buffer.h"

void acquisisci_accesso_scrittura_buffer(pthread_mutex_t *mutexbuffer, int *dati, pthread_cond_t *full) {
    if (pthread_mutex_lock(mutexbuffer) != 0) {
        perror("Errore nel bloccare il mutex");
        exit(EXIT_FAILURE);
    }
    while (*dati == PC_buffer_len) {
        if (pthread_cond_wait(full, mutexbuffer) != 0) {
            perror("Errore in pthread_cond_wait");
            exit(EXIT_FAILURE);
        }
    }
}

void rilascia_accesso_scrittura_buffer(pthread_mutex_t *mutexbuffer, int *dati, pthread_cond_t *empty) {
    *dati += 1;
    if (pthread_cond_signal(empty) != 0) {
        perror("Errore in pthread_cond_signal");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_unlock(mutexbuffer) != 0) {
        perror("Errore nel sbloccare il mutex");
        exit(EXIT_FAILURE);
    }
}

void acquisisci_accesso_lettura_buffer(pthread_mutex_t *mutexbuffer, int *dati, pthread_cond_t *empty) {
    if (pthread_mutex_lock(mutexbuffer) != 0) {
        perror("Errore nel bloccare il mutex");
        exit(EXIT_FAILURE);
    }
    while (*dati == 0) {
        if (pthread_cond_wait(empty, mutexbuffer) != 0) {
            perror("Errore in pthread_cond_wait");
            exit(EXIT_FAILURE);
        }
    }
}

void rilascia_accesso_lettura_buffer(pthread_mutex_t *mutexbuffer, int *dati, pthread_cond_t *full) {
    *dati -= 1;
    if (pthread_cond_signal(full) != 0) {
        perror("Errore in pthread_cond_signal");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_unlock(mutexbuffer) != 0) {
        perror("Errore nel sbloccare il mutex");
        exit(EXIT_FAILURE);
    }
}