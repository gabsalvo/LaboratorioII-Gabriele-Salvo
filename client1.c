#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HOST "127.0.0.1"
#define PORT 56181 
#define Max_sequence_length 2048

ssize_t writen(int fd, void *ptr, size_t n);

int main(int argc, char **argv) {
  sleep(2);
  if (argc != 2) {
    fprintf(stderr, "Inserisci il nome di un file di testo\n");
    exit(EXIT_FAILURE);
  }
  char *nomefile = argv[1];

  // Apertura file
  FILE *f = fopen(nomefile, "r");
  if (f == NULL) {
    perror("Errore apertura file");
    exit(EXIT_FAILURE);
  }

  int fd_skt = 0;
  struct sockaddr_in serv_addr;
  int e;
  int tmp;

  size_t len = 0;
  char *linea = NULL;

  while (1) {
    e = getline(&linea, &len, f);
    int dimensione = e;
    assert(dimensione < Max_sequence_length);
    if (e == -1)
      break;

    // Apertura socket
    if ((fd_skt = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("Errore creazione socket");
      exit(EXIT_FAILURE);
    }

    // Assegnazione indirizzo
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(HOST);

    // Apertura connessione
    if (connect(fd_skt, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("Errore apertura connessione");
      exit(EXIT_FAILURE);
    }

    // Comunicazione tipo di connessione
    tmp = htonl(0);
    if (write(fd_skt, &tmp, sizeof(tmp)) != sizeof(int)) {
      perror("Errore write");
      exit(EXIT_FAILURE);
    }

    // Invio dimensione sequenza
    int lendecodificata = htonl(dimensione);
    if (writen(fd_skt, &lendecodificata, sizeof(lendecodificata)) == -1) {
      perror("Errore invio dimensione della sequenza");
      exit(EXIT_FAILURE);
    }

    // Invio sequenza
    if (writen(fd_skt, linea, dimensione) == -1) {
      perror("Errore invio sequenza");
      exit(EXIT_FAILURE);
    }

    // Chiusura socket
    if (close(fd_skt) < 0) {
      perror("Errore chiusura socket");
      exit(EXIT_FAILURE);
    }
  }

  free(linea);

  // Chiusura file
  if (fclose(f) != 0) {
    perror("Errore chiusura file");
    exit(EXIT_FAILURE);
  }

  return 0;
}

ssize_t writen(int fd, void *ptr, size_t n) {
  size_t nleft = n;
  ssize_t nwritten;
  char *ptr_cast = (char *)ptr;

  while (nleft > 0) {
    if ((nwritten = write(fd, ptr_cast, nleft)) <= 0) {
      if (nleft == n)
        return -1; /* errore, ritorno -1 */
      else
        break; /* errore, ritorno quantità scritta finora */
    }
    nleft -= nwritten;
    ptr_cast += nwritten;
  }
  return (n - nleft); /* ritorno >= 0 */
}
