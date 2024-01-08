#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define HOST "127.0.0.1"
#define PORT 56181 
#define Max_sequence_length 2048

ssize_t writen(int fd, void *ptr, size_t n);
ssize_t readn(int fd, void *ptr, size_t n);

void *tbody(void *argv) {
  FILE *f = fopen((char *)argv, "r");
  if (f == NULL) {
    perror("Errore apertura file");
    pthread_exit(NULL);
  }

  int fd_skt = 0;
  struct sockaddr_in serv_addr;
  int e;
  int tmp;

  if ((fd_skt = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Errore creazione socket");
    pthread_exit(NULL);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = inet_addr(HOST);

  if (connect(fd_skt, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Errore apertura connessione");
    pthread_exit(NULL);
  }

  tmp = htonl(1);
  if (write(fd_skt, &tmp, sizeof(tmp)) != sizeof(int)) {
    perror("Errore write");
    pthread_exit(NULL);
  }

  int numerosequenzespedite = 0;
  size_t len = 0;
  char *linea = NULL;

  while (1) {
    e = getline(&linea, &len, f);
    int dimensione = e;
    if (e == -1)
      break;
    assert(dimensione < Max_sequence_length);

    int lendecodificata = htonl(dimensione);
    if (writen(fd_skt, &lendecodificata, sizeof(lendecodificata)) == -1) {
      perror("Errore invio dimensione della sequenza");
      pthread_exit(NULL);
    }

    if (writen(fd_skt, linea, dimensione) == -1) {
      perror("Errore invio sequenza");
      pthread_exit(NULL);
    }

    numerosequenzespedite++;
  }

  int zerodecodificato = htonl(0);
  if (writen(fd_skt, &zerodecodificato, sizeof(zerodecodificato)) == -1) {
    perror("Errore invio 0 prima dell'ultima sequenza");
    pthread_exit(NULL);
  }

  if (writen(fd_skt, "", 0) == -1) {
    perror("Errore invio sequenza");
    pthread_exit(NULL);
  }

  int numerosequenzedadecodificare;
  if (readn(fd_skt, &numerosequenzedadecodificare, sizeof(numerosequenzedadecodificare)) == -1) {
    perror("Errore ricezione numero sequenze");
    pthread_exit(NULL);
  }

  int numerosequenze = ntohl(numerosequenzedadecodificare);
  if (numerosequenze != numerosequenzespedite) {
    fprintf(stderr, "Il numero di sequenze spedite da client2 è diverso dal numero di sequenze ricevute dal server\n");
    pthread_exit(NULL);
  }

  if (close(fd_skt) < 0) {
    perror("Errore chiusura socket");
    pthread_exit(NULL);
  }

  free(linea);
  if (fclose(f) != 0) {
    perror("Errore chiusura file");
    pthread_exit(NULL);
  }
  
  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  sleep(1);
  if (argc < 2) {
    fprintf(stderr, "Inserisci uno o più file di testo da cui leggere\n");
    exit(EXIT_FAILURE);
  }

  pthread_t client2[argc - 1];
  for (int i = 0; i < argc - 1; i++) {
    char *nomefile = argv[i + 1];
    if (pthread_create(&client2[i], NULL, tbody, nomefile) != 0) {
      perror("Errore pthread_create");
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < argc - 1; i++) {
    if (pthread_join(client2[i], NULL) != 0) {
      perror("Errore pthread_join");
      exit(EXIT_FAILURE);
    }
  }

  return 0;
}

ssize_t writen(int fd, void *ptr, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *ptr_cast = (char *)ptr;

    while (nleft > 0) {
        if ((nwritten = write(fd, ptr_cast, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;  // chiamata interrotta da signal, riprova
            else
                return -1;     // errore nella scrittura
        }
        nleft -= nwritten;
        ptr_cast += nwritten;
    }
    return n;  // numero esatto di byte scritti
}


ssize_t readn(int fd, void *ptr, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *ptr_cast = (char *)ptr;

    while (nleft > 0) {
        if ((nread = read(fd, ptr_cast, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;   // chiamata interrotta, riprova
            else
                return -1;  // errore nella lettura
        } else if (nread == 0) {
            break;  // EOF raggiunto
        }
        nleft -= nread;
        ptr_cast += nread;
    }
    return (n - nleft);  // numero di byte letti, potrebbe essere minore di n se EOF è raggiunto
}
