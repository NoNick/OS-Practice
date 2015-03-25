all:
	make -C lib/
	make -C cat/
	make -C revwords/
	make -C filter/

clean:
	make clean -C lib/
	make clean -C cat/
	make clean -C revwords/
	make clean -C filter/
