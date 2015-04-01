CC = gcc
SRC = cat.c
C_FLAGS = -fPIC -L../lib/ -I../lib/ -lhelpers

all: cat

cat: $(SRC)
	$(CC) -o cat $(SRC) $(C_FLAGS)

clean:
	rm -rf cat

