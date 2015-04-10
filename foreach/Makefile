CC = gcc
SRC = foreach.c
C_FLAGS = -fPIC -g -L../lib -I../lib -lhelpers -lbufio

all: foreach

foreach: $(SRC)
	$(CC) -o foreach $(SRC) $(C_FLAGS)

clean:
	rm -rf foreach

