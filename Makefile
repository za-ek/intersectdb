CC = gcc
CFLAGS = -Wall
OBJFILES = main.o utils.o db2.o
TARGET = intersectdb

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)

main.o: src/db2.h src/utils.h
	$(CC) $(CFLAGS) -c src/main.c

utils.o: src/utils.h
	$(CC) $(CFLAGS) -c src/utils.c

db2.o: src/db2.h src/utils.h
	$(CC) $(CFLAGS) -c src/db2.c

clean:
	rm -f $(OBJFILES) $(TARGET) *~
