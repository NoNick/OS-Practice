CC = gcc
SRC_HELPERS = helpers.h helpers.c
SRC_BUFIO = bufio.h bufio.c
C_FLAGS = -fPIC -shared -O0 -g

all: libhelpers.so libbufio.so clean_o

libhelpers.so: $(SRC_HELPERS)
	$(CC) -o libhelpers.so $(SRC_HELPERS) $(C_FLAGS)

libbufio.so: $(SRC_BUFIO)
	$(CC) -o libbufio.so $(SRC_BUFIO) $(C_FLAGS) -L. -lhelpers

clean_o:
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf *.so
