all:
	make -C lib/
	make -C cat/

clean:
	make clean -C lib/
	make clean -C cat/
