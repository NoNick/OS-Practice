all:
	make -C lib/
	make -C cat/
	make -C revwords/

clean:
	make clean -C lib/
	make clean -C cat/
	make clean -C revwords/
