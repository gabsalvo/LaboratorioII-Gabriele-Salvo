# Definizione del compilatore e dei flag di compilazione
CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O -g -D_POSIX_C_SOURCE=200809L
LDLIBS = -lm -lrt -pthread

EXECS = archivio client1 client2

all: $(EXECS)

# Regola per la creazione degli eseguibili
$(EXECS): %: %.o buffer.o writer.o reader.o manager.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Regola per la creazione di file oggetto
%.o: %.c
	$(CC) $(CFLAGS) -c $<

archivio.o: archivio.c buffer.h writer.h reader.h manager.h
buffer.o: buffer.c buffer.h
writer.o: writer.c buffer.h
manager.o: manager.c buffer.h writer.h reader.h
reader.o: reader.c buffer.h writer.h

# Regola per rimuovere i file oggetti, gli eseguibili ed i file di log
clean: 
	rm -f *.o $(EXECS) capolet caposc
	rm -f *.log
