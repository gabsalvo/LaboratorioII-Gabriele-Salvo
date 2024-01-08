# Archivio

La struttura del progetto, suddivisa in quattro librerie distinte (writer.h, reader.h, buffer.h, manager.h), riflette un approccio modulare alla programmazione in C, mirato a ottimizzare l'organizzazione del codice e a facilitare la manutenzione e la scalabilità del sistema

## 1. Buffer (buffer.h)

Questa libreria gestisce il buffer produttori-consumatori, una struttura dati fondamentale per il coordinamento tra i thread scrittori e lettori. Implementando funzioni per acquisire e rilasciare l'accesso in lettura e scrittura, questa libreria fornisce un meccanismo di sincronizzazione essenziale che previene condizioni di gara e assicura l'integrità dei dati. La scelta di separare la gestione del buffer in una libreria dedicata rispecchia i principi di singola responsabilità e modularità, permettendo di mantenere il codice organizzato e facilmente manutenibile.

## 2. Writer (writer.h)

La libreria writer.h definisce le strutture e le funzioni specifiche per i thread scrittori. Gestisce la creazione di nuovi dati, la loro inserzione nel buffer e la sincronizzazione con altri thread. La separazione dei compiti degli scrittori in una libreria dedicata permette una maggiore chiarezza nel codice e facilita la gestione di specifiche funzionalità legate agli scrittori.

## 3. Reader (reader.h)

Analogamente a writer.h, reader.h si concentra sui thread lettori. Questa libreria definisce come i lettori accedono ai dati nel buffer e come questi vengono processati. Separare i lettori dagli scrittori in librerie differenti aiuta a mantenere il codice organizzato, rendendo più semplice gestire e ottimizzare ogni parte del sistema in modo indipendente.

## 4. Manager (manager.h)

Questa libreria funge da supervisore o coordinatore del sistema. Contiene la logica per la gestione dei vari thread, la loro sincronizzazione e la gestione dei segnali. La presenza di una libreria dedicata al "manager" del sistema consente di avere un punto centrale di controllo, rendendo più chiare le interazioni tra i vari componenti e facilitando la gestione degli aspetti globali dell'applicazione.

# Server

Nel sviluppare server.py, ho mirato a creare un server versatile e affidabile. Utilizzando il modulo argparse, ho permesso agli utenti di personalizzare facilmente i parametri di esecuzione, come il numero di thread e l'opzione di eseguire il programma archivio con Valgrind, migliorando così la flessibilità e la facilità di debug.

Ho implementato la creazione di pipe con nome per una comunicazione efficiente con il processo archivio. Successivamente, ho configurato un socket server che utilizza ThreadPoolExecutor per gestire le connessioni in entrata in modo asincrono, assegnando un thread dedicato a ogni connessione.

Le funzioni recv_allc1 e recv_allc2 gestiscono la ricezione dei dati dai client, scrivendo le informazioni ricevute nelle pipe in mutua esclusione. Questo assicura una sincronizzazione efficace e riduce il rischio di conflitti.

In caso di eccezioni o interruzioni, ho implementato un meccanismo di pulizia che chiude le pipe, rimuove le connessioni e invia un segnale SIGTERM a archivio, garantendo una chiusura ordinata e sicura del server. Ho inoltre utilizzato lock separati per la scrittura sulle pipe e sui file di log per ottimizzare la gestione della concorrenza e migliorare le prestazioni del server.

# Client1 e Client2

n client1.c e client2.c, ho sviluppato due client distinti per interagire con il server. Entrambi i client hanno il compito di leggere sequenze di dati da file di testo e inviarle al server, ma differiscono nella modalità di comunicazione e nel trattamento delle risposte del server.

## client1.c

- Lettura del File: Il client apre un file di testo e legge sequenze di dati. Per ogni sequenza, stabilisce una nuova connessione con il server.

- Invio dei Dati: Ogni sequenza viene inviata insieme alla sua lunghezza. Utilizzo la funzione writen per assicurarmi che tutti i dati vengano inviati correttamente, gestendo anche eventuali interruzioni.

- Connessione e Chiusura Socket: Per ogni sequenza, il client apre una nuova socket, invia i dati e chiude la socket. Questo modello di "usa e getta" semplifica la gestione delle connessioni, ma potrebbe non essere l'approccio più efficiente in termini di prestazioni.

## client2.c

- Gestione Multithread: Utilizzo i thread per gestire più file di input in parallelo. Ogni thread legge da un file diverso e invia i dati al server, permettendo un'elaborazione più efficiente e parallela.

- Comunicazione Continua: A differenza di client1, client2 mantiene aperta la connessione per l'intera durata dell'invio di tutte le sequenze da un singolo file, inviando un segnale di terminazione alla fine.

- Ricezione e Verifica Risposte: Dopo aver inviato tutte le sequenze, client2 attende una risposta dal server che indica il numero di sequenze ricevute, confrontandolo con il numero di sequenze inviate per verificare la correttezza della trasmissione.
