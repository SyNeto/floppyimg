CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

# SRC = src/fat12lib.c src/main.c
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = bin/floppyimg

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)
