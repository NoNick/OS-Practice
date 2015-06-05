CC = gcc
SRC1 = forking.c
SRC2 = polling.c
C_FLAGS = -O0 -g -fPIC -L../lib -I../lib -lbufio -lhelpers

all: forking polling

forking: $(SRC1)
	$(CC) -o forking $(SRC1) $(C_FLAGS)

polling: $(SRC2)
	$(CC) -o polling $(SRC2) $(C_FLAGS)

clean:
	rm -rf forking
	rm -rf polling

