.PHONY: all

all: client server

client:
	make -f makefile_client

server:
	make -f makefile_server

.PHONY: clean

clean:
	make -f makefile_client clean
	make -f makefile_server clean
