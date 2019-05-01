#ifndef WTFSERVER_H
#define WTFSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <openssl/sha.h>
#include <sys/sendfile.h>


struct manilist{
	char modded[1];
	int version;
	char filename[512];
	char hash[33];
	struct manilist* next;
};


void sCreate(int);
int clearDir(char*);
void sDestroy(int);



#endif
