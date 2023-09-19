# vim: set autoindent noexpandtab tabstop=4 shiftwidth=4
CC = gcc

# Compiler flags
CFLAGS = -Wall -std=c11 -pthread -lrt -pedantic -g

# RM
RM = rm -v

# Object directory
OBJ = ../obj

# Output directory
OUT = ../bin

# Dependancies directory
LIB = ./lib

OBJECTS = commands.o hashtable.o linkedlist.o libdev.o doublylinkedlist.o


%.o: ../src/lib/%.c ../src/lib/%.h
	$(CC) $(CFLAGS) -o $(OBJ)/$@ -c $<

server: server.c
	$(CC) $(CFLAGS) -o $(OUT)/server server.c \
	$(OBJ)/commands.o $(OBJ)/hashtable.o $(OBJ)/linkedlist.o \
	$(OBJ)/doublylinkedlist.o $(OBJ)/libdev.o

client: client.c
	$(CC) $(CFLAGS) -o $(OUT)/client client.c \
	$(OBJ)/commands.o $(OBJ)/hashtable.o $(OBJ)/linkedlist.o \
	$(OBJ)/doublylinkedlist.o $(OBJ)/libdev.o

.PHONY: all clean

all: $(OBJECTS) server client 

clean:
	$(RM) $(OBJ)/*.o $(OUT)/server $(OUT)/client || true