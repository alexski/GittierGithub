#ifndef WTF_H
#define WTF_H

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

struct data{
    char ip[128];
    char port[128];
};

struct manilist{
	char modded[1];
	int version;
	char filename[512];
	char hash[33];
	struct manilist* next;
};

void config(char*, char*);
int config_check();
void build_info(struct data*);

void create(int, char*, struct data*);

void destroy(int, char*, struct data*);

void freeMani(struct manilist*);
void updateMani(char*, struct manilist**, int);
int addManilist(char*, char*, struct manilist**);
int buildManilist(char*, struct manilist**);
unsigned char* fileHash(char*);
void add(char* proj, char* filename);
int removeManilist(char*, struct manilist**);
void removeFile(char*, char*);

#endif
