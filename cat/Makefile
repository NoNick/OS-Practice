CC = gcc
SRC = cat.c
C_FLAGS = -fPIC -L../lib/ -I../lib/ -lhelpers

all: cat

cat:
	$(CC) -o cat $(C_FLAGS) $(SRC)

clean:
	rm -rf cat

