CC = gcc
SRC = revwords.c
C_FLAGS = -fPIC -L../lib -I../lib -lhelpers

all: revwords

revwords: $(SRC)
	$(CC) -o revwords $(SRC) $(C_FLAGS)

clean:
	rm -rf revwords

