client: lisek-client.o
	gcc -o client lisek-client.o

lisek-client.o: lisek-client.c thread.h
	gcc -c lisek-client.c

clean:
	rm -f *.o client
