#include "WTF.h"

void build_info(struct data* conn_info){
    char c[1];
    int i, pos, datapoint;
    char buffer[128];
    buffer[0] = '\0';
    
    int config = open("./.configure", O_RDONLY, 00666);
    
    pos = 0;
    datapoint = 1; // 1 is looking for ip and 2 is looking for port
    int fileSize = lseek(config, 0, SEEK_END);
    
    lseek(config, 0, SEEK_SET);
    
    for(i = 0; i < fileSize; ++i){
        read(config, c, 1);
        
        if(c[0] == '\n'){
            buffer[pos] = '\0';
            
            if(datapoint == 1){ // looking for ip
                strcpy(conn_info->ip, buffer);
                datapoint = 2;
            }else{
                strcpy(conn_info->port, buffer);
            }
            
            buffer[0] = '\0';
            pos = 0;
        }else{
            buffer[pos] = c[0];
            ++pos;
        }
    }
    
    close(config);
    return;
};

void config(char* ip, char* port){
    int file = 0;
    
    file = open(".configure", O_RDWR | O_CREAT, 00666);

    write(file, ip, strlen(ip));
    write(file, "\n", 1);
    write(file, port, strlen(port));
    write(file, "\n", 1);
    close(file);

    return;
};

int config_check(){
    struct stat* buff = malloc(sizeof(struct stat));
    int configFile = stat("./.configure", buff);

    if(configFile != 0){
        fprintf(stderr, "Error: Configuration file does not exist. Please configure connection.\n");
        return 1;
    }
    
    free(buff);
    return 0;
}

void create(int network, char* proj){
    char servRes[256];
    servRes[0] = '\0';
    
    send(network, proj, sizeof(proj), 0);
    recv(network, &servRes, sizeof(servRes), 0);
    printf("%s\n", servRes);
    
    return;
};












