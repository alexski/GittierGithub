all: WTF.c WTFlib.o
	gcc WTF.c WTFlib.o -o WTF -lcrypto -lssl

WTFlib.o: WTFlib.c
	gcc -c WTFlib.c

clean:
	rm WTF; rm WTFlib.o
