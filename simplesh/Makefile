CC = gcc
SRC = simplesh.c
C_FLAGS = -O0 -g -fPIC -L../lib/ -I../lib/ -lhelpers

all: simplesh

simplesh: $(SRC)
	$(CC) -o simplesh $(SRC) $(C_FLAGS)

clean:
	rm -rf simplesh

