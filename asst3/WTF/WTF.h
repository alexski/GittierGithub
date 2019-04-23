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

void config(char*, char*);
int config_check();
void build_info(struct data*);
void create(int, char*, struct data*);

#endif
