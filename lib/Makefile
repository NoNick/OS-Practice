CC = gcc
SRC = helpers.h helpers.c
C_FLAGS = -fPIC -shared

all: libhelpers.so clean_o

libhelpers.so:
	$(CC) -o libhelpers.so $(C_FLAGS) helpers.c helpers.h

clean_o:
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf *.so
