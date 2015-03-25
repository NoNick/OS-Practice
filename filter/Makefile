CC = gcc
SRC = filter.c
C_FLAGS = -fPIC -L../lib -I../lib -lhelpers

all: filter

filter:
	$(CC) -o filter $(C_FLAGS) $(SRC)

clean:
	rm -rf filter

