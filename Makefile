CC = gcc
CFLAGS = -W -Wall -g -pthread
OBJ = main.o house.o ghost.o hunter.o helpers.o
EXE = simulation

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

main.o: main.c defs.h helpers.h
	$(CC) $(CFLAGS) -c main.c

house.o: house.c defs.h helpers.h
	$(CC) $(CFLAGS) -c house.c

ghost.o: ghost.c defs.h helpers.h
	$(CC) $(CFLAGS) -c ghost.c

hunter.o: hunter.c defs.h helpers.h
	$(CC) $(CFLAGS) -c hunter.c

helpers.o: helpers.c helpers.h defs.h
	$(CC) $(CFLAGS) -c helpers.c

clean:
	rm -f $(OBJ) $(EXE)