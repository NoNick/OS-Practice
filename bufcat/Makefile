CC = gcc
SRC = bufcat.c
C_FLAGS = -fPIC -L../lib -I../lib -lbufio -lhelpers -g

all: bufcat

bufcat:
	$(CC) -o bufcat $(SRC) $(C_FLAGS)

clean:
	rm -rf bufcat

