all: WTFserver.c serverlib.o
	gcc -g WTFserver.c serverlib.o -o WTFserver -lssl -lcrypto

serverlib.o: serverlib.c
	gcc -c serverlib.c

clean:
	rm WTFserver; rm serverlib.o
