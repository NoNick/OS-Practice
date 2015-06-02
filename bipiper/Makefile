CC = gcc
SRC = forking.c
C_FLAGS = -fPIC -L../lib -I../lib -lbufio -lhelpers

all: forking

forking: $(SRC)
	$(CC) -o forking $(SRC) $(C_FLAGS)

clean:
	rm -rf forking

