CC = gcc
SRC = filter.c
C_FLAGS = -fPIC -L../lib -I../lib -lhelpers

all: filter

filter: $(SRC)
	$(CC) -o filter $(SRC) $(C_FLAGS)

clean:
	rm -rf filter

