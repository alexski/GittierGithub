all: WTFserver.c serverlib.o
	gcc WTFserver.c serverlib.o -o WTFserver

serverlib.o: serverlib.c
	gcc -c serverlib.c

clean:
	rm WTFserver; rm serverlib.o
