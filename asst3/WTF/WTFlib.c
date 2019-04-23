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

void create(int network, char* proj, struct data* connInfo){
    char response[256];
    char recvd[3];
    char path[256];
    recvd[0] = '\0';
    response[0] = '\0';
    DIR* projDir;
    int mani, mani_size;
    int bytesReceived = 0;
    char ms[10];
    char data[256];
    
    send(network, proj, sizeof(proj), 0);
    recv(network, &recvd, sizeof(recvd), 0);
    
    if(strcmp(recvd, "OK") != 0){
    	printf("Error: Project name was not successfully sent to server.\n");
    	return;
    }
    
    recv(network, &response, sizeof(response), 0);
    
    // server has created a dir for the proj and a .Manifest
    if(strcmp(response, "success") == 0){
    	
    	// build a dir for the proj on the client side
    	strcpy(path, "./");
    	strcat(path, proj);
    	projDir = opendir(path);
    	
    	
    	if(projDir){
    		fprintf(stderr, "Error: Project already exists on the client side.\n");
    		return;
    	}else if(errno == ENOENT){ // proj dir does not exist therefore one must be created
    		
    		// create a proj dir on client side
			mkdir(path, ACCESSPERMS);
    		
			// Initialize .Manifest file in project folder
			strcat(path, "/.Manifest");
			mani = open(path, O_RDWR | O_CREAT, 00666);
			
			// receive data from .Manifest from server side to put into .Manifest client side
			recv(network, data, sizeof(data), 0);
			write(mani, data, 2);

			close(mani);
    		
    	}else{ // something went wrong with opendir()
    		fprintf(stderr, "Error: opendir() failed for project's directory on client side.\n");
    		return;
    	}
    
    // there was some error that stopped the server from going through with the function
    }else{
    	fprintf(stderr, "%s\n", response);
    	return;
    }
    
    return;
};












