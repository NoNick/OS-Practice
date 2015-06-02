CC = gcc
SRC = filesender.c
C_FLAGS = -fPIC -L../lib -I../lib -lbufio -lhelpers -g -O0

all: filesender

filesender: $(SRC)
	$(CC) -o filesender $(SRC) $(C_FLAGS)

clean:
	rm -rf filesender

