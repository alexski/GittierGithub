#include "WTFserver.h"

// USE man socket IN TERMINAL FOR MORE INFORMATION

/* Frees manilist linked list */
void sfreeMani(struct manilist* front){
	struct manilist* ptr = front;
	struct manilist* prev;
	
	while(ptr != NULL){
		prev = ptr;
		ptr = ptr->next;
		free(prev);
	}

	return;
};


int sbuildManilist(char* path, struct manilist** node){
    char c[1];
    int i;
    int first = 0;
    int pos = 0;
    int datapoint = 0;
    int mani_version = -1;
    char buffer[1024];
    buffer[0] = '\0';
    c[0] = '\0';
    struct manilist* ptr;
    struct manilist* newNode;
    
    int mani = open(path, O_RDONLY, 00666);
    int fileSize = lseek(mani, 0, SEEK_END);
    
    lseek(mani, 0, SEEK_SET);
	
	read(mani, c, 1);
	
    while(c[0] != '\n'){
    	buffer[pos] = c[0];
		++pos;
    	
    	read(mani, c, 1);
    }
    buffer[pos] = '\0';
    
    if(buffer[0] == '\0') return -1;
    
    mani_version = atoi(buffer);
    
    if(mani_version == 0){
    	close(mani);
    	return mani_version;
    }
    
    i = pos;
    pos = 0;
    buffer[0] = '\0';
    
    if(*node == NULL){
    	*node = (struct manilist*) malloc(sizeof(struct manilist));
    }
	ptr = *node;
    
    for(i; i < fileSize; ++i){
        read(mani, c, 1);
        
        if(c[0] == '\t'){
            buffer[pos] = '\0';
            
            switch(datapoint){
            
				case 0: // modded
					if(first == 1){
						newNode = malloc(sizeof(struct manilist));
        				ptr->next = newNode;
        				ptr = newNode;
					}
					first = 1;
            		ptr->modded[0] = buffer[0];
            		++datapoint;
            		break;
            		
            	case 1: // file version
            		ptr->version = atoi(buffer);
            		++datapoint;
            		break;
            		
            	case 2: // filename
            		strcpy(ptr->filename, buffer);
            		++datapoint;
            		break;
            }
            
            buffer[0] = '\0';
            pos = 0;
            
        }else if(c[0] == '\n'){ // hashcode
        	if(buffer[0] == '\0'){
        		break;
        	}
        	buffer[pos] = '\0';
        	
        	strcpy(ptr->hash, buffer);
        	
        	ptr->next = NULL;
        	datapoint = 0;

            buffer[0] = '\0';
            pos = 0;
    		
        }else{
            buffer[pos] = c[0];
            ++pos;
        }
    }
    
    close(mani);
	return mani_version;
};


void sCurrentVersion(int sock){
	char proj[256];
	char response[256];
	char sent[3] = "OK";
	char path[512] = "./repository";
	struct stat* manistat;
	struct manilist* front;
	int mani_version;
	char c[1];
	char buffer[256];
	char fv[128];
	buffer[0] = '\0';
	c[0] = '\0';
	
	proj[0] = '\0';
	response[0] = '\0';
	
	DIR* repos = opendir(path);
	int mani;
	
	recv(client, &proj, sizeof(proj), 0);
	send(client, sent, sizeof(sent), 0);
	
	if(proj[0] == '\0'){
		fprintf(stderr, "Error: Did not receive project name from client\n");
		return;
	}

	if(repos){ // if repository directory exists send manifest contents to client
		strcat(path, "/");
		strcat(path, proj);
		DIR* projDir = opendir(path);
		
		if(projDir){
			strcpy(response, "success");
			send(client, response, sizeof(response), 0);
			
			char meta[strlen(path) + 11];
			strcat(meta, path);
			strcat(path, "/.metadata");
			int md = open(meta, O_RDWR, 00666);
			
			read(md, c, 1);
			while(c[0] != '\n'){
				buffer[pos] = c[0];
				++pos;
				
				read(md, c, 1);
			}
			buffer[pos] = '\0';
			close(md);
			
			strcat(path, "/");
			strcat(path, buffer);
			buffer[0] = '\0';
			
			strcat(path, "/.Manifest");
			
			mani_version = sbuildManilist(path, &front);
			
			if(mani_version == 0){
				strcpy(response, "none");
				send(client, response, sizeof(response), 0);
			
			}else if(mani_version == -1){
				fprintf(stderr, "Error: .Manifest file is not formatted correctly.");
				strcpy(response, "Error: .Manifest file is not formatted correctly.");
				send(client, response, sizeof(response), 0);
				
			}else{
				strcpy(response, "full");
				send(client, response, sizeof(response), 0);
				
				ptr = front;
				
				while(ptr != NULL){
					strcpy(buffer, ptr->filename);
					strcat(buffer, " ");
					sprintf(fv, "%d", ptr->version);
					strcat(buffer, fv);
					strcpy(response, buffer);
					send(client, response, sizeof(response), 0);
					ptr = pre->next;
				}
				
				strcpy(response, "done");
				send(client, response, sizeof(response), 0);
			}
			
			sfreeMani(front);
			return;
			
		}else if(errno == ENOENT){ // proj dir does not exist
			
			strcpy(response, "Error: Project directory does not exist on the server side.");
			fprintf(stderr, "%s\n", response);
			send(client, response, sizeof(response), 0);
			
		}else{ // something wrong happened with opendir() in project dir
			fprintf(stderr, "Error: opendir() failed for project's directory on server side.\n");
			strcpy(response, "Error: opendir() failed for project's directory server side.");
			send(client, response, sizeof(response), 0);
		}
		
	}else if(errno == ENOENT){ // repository folder does not exist
		
		fprintf(stderr, "Error: Repository directory does not exist on server side.\n");
		strcpy(response, "Error: Repository directory does not exist on server side.");
		send(client, response, sizeof(response), 0);
		
	}else{ // something went wrong with opendir()
		fprintf(stderr, "Error: opendir() failed for repository directory on server side.\n");
		strcpy(response, "Error: opendir() failed for repository directory on server side.");
		send(client, response, sizeof(response), 0);
	}
	
	// close repos dir
	closedir(repos);
	return;
	

	return;
};

int main(int argc, char** argv){
    int portno;
    int client_socket, server_socket;
    char server_message[256] = "Connected to server.";
    char sent[3] = "OK";
    char client_response[256];
    char client_command[128];
    //pthread_t tid;
    
    
    if(argc != 2){
        printf("Error: Insufficient Number of Arguments.\n");
        printf("Please provide a port number to connect to.\n");
        return -1;
    }else{
        portno = atoi(argv[1]);
    }
    
    // create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portno);
    server_address.sin_addr.s_addr = INADDR_ANY;
    
    // bind the socket to our specified IP and port
    if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0){
    	fprintf(stderr, "Error: Binding error\n");
    	return -1;
    }
    
    listen(server_socket, 5);

		client_socket = accept(server_socket, NULL, NULL);
		
		// send the message
		send(client_socket, server_message, sizeof(server_message), 0);
		
		if(recv(client_socket, &client_command, sizeof(client_response), 0) != -1){
			send(client_socket, sent, sizeof(sent), 0);
			if(strcmp(client_command, "create") == 0){
				sCreate(client_socket);
			}else if(strcmp(client_command, "destroy") == 0){
				sDestroy(client_socket);
			}else if(strcmp(client_command, "currentversion") == 0){
				sCurrentVersion(client_socket);
		}else{
			fprintf(stderr, "Error: Message not received from client.\n");
		}
	
	close(client_socket);
	return 0;
}
