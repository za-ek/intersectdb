CC = gcc
CFLAGS = -g
objects = main.o utils.o db2.o

edit : $(objects)
	cc -o edit $(objects)

main.o : utils.h db2.h
utils.o : utils.h
db2.o : db2.h

.PHONY: clean
clean :
	rm edit $(objects)

build: main.c db2.c utils.c
	$(CC) $(CFLAGS) -o main *.o
