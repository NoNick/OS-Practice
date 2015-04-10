all:
	make -C lib/
	make -C cat/
	make -C revwords/
	make -C filter/
	make -C bufcat/
	make -C foreach/

clean:
	make clean -C lib/
	make clean -C cat/
	make clean -C revwords/
	make clean -C filter/
	make clean -C bufcat/
	make clean -C foreach/
