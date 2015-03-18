CC = gcc
SRC = revwords.c
C_FLAGS = -fPIC -L../lib -I../lib -lhelpers

all: revwords

revwords:
	$(CC) -o revwords $(C_FLAGS) $(SRC)

clean:
	rm -rf revwords

