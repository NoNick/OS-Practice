CC = gcc
SRC = filter.c
C_FLAGS = -fPIC -L../lib -I../lib -lhelpers

all: filter

filter:
	$(CC) -o filter $(SRC) $(C_FLAGS)

clean:
	rm -rf filter

