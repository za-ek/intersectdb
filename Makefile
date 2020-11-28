TARGET_DIR=build
CC = gcc
CFLAGS = -Wall
TARGET = $(TARGET_DIR)/intersectdb

OBJFILES = $(TARGET_DIR)/main.o $(TARGET_DIR)/utils.o $(TARGET_DIR)/db2.o $(TARGET_DIR)/cmd.o

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)

$(TARGET_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJFILES) $(TARGET) *~

rebuild: clean all