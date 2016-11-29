all: win nix

win:
	make -f makefile.win

nix:
	make -f makefile.nix

clean:
	rm -f server.o client.o
