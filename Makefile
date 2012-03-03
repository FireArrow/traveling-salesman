#!/bin/bash

# Makefile make me fast and accurate

CC = gcc
EXE = traveling-salesman
EXE_ARGS = ./test_input_01.in

CFLAGS = -Wall -g -Wno-error -std=c99 -fopenmp
LFLAGS = 

SRC = $(EXE).c
OBJ = $(SRC:.c=.o)

.PHONY: clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) $(LFLAGS) -o $(EXE)

$(OBJ): $(SRC)
	$(CC) -c $(SRC) $(CFLAGS)

clean: 
	rm -rf $(OBJ) $(EXE)

run_test: $(EXE)
	./$(EXE) $(EXE_ARGS)

# End of Makefile



