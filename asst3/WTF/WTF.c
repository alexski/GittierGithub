#include "WTF.h"

/*  CLIENT COMMANDS
configure | checkout | update | upgrade
commit | push | create | create | destroy
add | remove | currentversion | history
rollback  */

void updateMani(char* manipath, struct manilist* front, int mani_version){
	struct manilist* ptr = front;
	int mani = open(manipath, O_RDWR, 00666);
	char mv[128];
	char fv[128];
	
	
	// write updated mani version
	mani_version++;
	sprintf(mv, "%d", mani_version);
	write(mani, mv, strlen(mv));
	write(mani, "\n", 1);
	
	while(ptr != NULL){
		write(mani, ptr->modded, 1);
		write(mani, "\t", 1);
		
		sprintf(fv, "%d", ptr->version);
		write(mani, fv, strlen(fv));
		write(mani, "\t", 1);
		
		write(mani, ptr->filename, strlen(filename));
		write(mani, "\t", 1);
		
		write(mani, ptr->hash, strlen(ptr->hash));
		write(mani, "\n", 1);
		
		ptr = ptr->next;
	}
	
	close(mani):
	return;
};

/* Checks if file was in the manifest and adds to end of list of its not. */
int addManilist(char* filename, char* hash, struct manilist* front){
	struct manilist* ptr = front;
	struct manilist* prev = NULL;
	
	while(ptr != NULL){
		if(strcmp(ptr->filename, filename) == 0){ // file is in manilist
			
			if(strcmp(ptr->hash, hash) == 0){ // file is already in mani and updated
				return 1;
				
			}else{ // file is in mani, but needs to be updated
				strcpy(ptr->hash, hash);
				ptr->version = ptr->version + 1;
				ptr->modded = '1';
			}
			
			return 0;
		}else{
			prev = ptr;
			ptr = ptr->next;
		}
	}

	ptr = (struct manilist*) malloc(sizeof(manilist));
	ptr->modded = '1';
	ptr->version = 1;
	strcpy(ptr->filename, filename);
	strcpy(ptr->hash, hash);
	
	if(prev == NULL){
		front = ptr;
	}else{
		prev->next = ptr;
	}
	
	return 0;
};


int buildManilist(char* path, struct manilist* front){
    char c[1];
    int i;
    int pos = 0;
    int datapoint = 0;
    char buffer[128];
    buffer[0] = '\0';
    struct manilist* ptr;
    struct manilist* newNode;
    
    int mani = open("./.Manifest", O_RDONLY, 00666);
    int fileSize = lseek(mani, 0, SEEK_END);
    
    lseek(mani, 0, SEEK_SET);
    
    while(c[0] == '\n'){
    	read(mani, c, 1);
    	
    	buffer[pos] = c[0];
    	++pos;
    }
    buffer[pos] = '\0';
    
    int mani_version = atoi(buffer);
    
    if(mani_version == 0){
    	close(mani);
    	return mani_version;
    }
    
    i = pos;
    pos = 0;
    buffer[0] = '\0';
    ptr = (struct manilist*) malloc(sizeof(struct manilist));
    front = ptr;
    
    for(i; i < fileSize; ++i){
        read(config, c, 1);
        
        if(c[0] == '\t'){
            buffer[pos] = '\0';
            
            switch(datapoint){
            
				case 0: // modded
            		front->modded = buffer[0];
            		++datapoint;
            		break;
            		
            	case 1: // file version
            		front->version = atoi(buffer);
            		++datapoint;
            		break;
            		
            	case 2: // filename
            		strcpy(front->filename, buffer);
            		++datapoint;
            		break;
            }
            
            buffer[0] = '\0';
            pos = 0;
            
        }else if(c[0] == '\n'){ // hashcode
        
    		front->hash = buffer;
    		if((i + 1) >= fileSize){
    			ptr->next = NULL;
    			break;
    		}else{
    			struct manilist* node = malloc(sizeof(struct manilist));
        		ptr->next = node;
        		ptr = node;
        		datapoint = 0;
    		}

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

/* ./WTF add <project name> <filename>
The add command will fail if the project does not exist on the client. The client 
will add an entry for the file to its own .Manifest with a new version number 
and hashcode.
(It is not required, but it may speed things up/make things easier 
for you if you add a code in the .Manifest to signify that this file was added 
locally and the server hasn't seen it yet) */
void add(char* proj, char* filename){
	struct stat fileInfo;
	DIR* projDir = opendir(proj);
	stat(filename, &fileInfo);
	int mani, mani_version;
	char* manipath;
	
	if(projDir){ // project directory exists

		// checks if filename is valid
		if(!S_ISREG(itemstat.st_mode)){
			fprintf(stderr, "Error: Invalid filename.\n");
			return;
		}
		
		// build the path for the .Manifest
		manipath = (char*) malloc((sizeof(proj) + 20) * sizeof(char));
		strcpy(manipath, proj);		
		strcat(manipath, "/.Manifest");
		struct manilist* mFront = NULL;
		
		// get hash for file
		
		// build a linked list of all the files in the .Manifest
		mani_version = buildManilist(manipath, mFront);
		
		if(maniversion == 0){ // .Manifest version is 0 and file is first file to be added
			mani = open(manipath, O_RDWR, 00666);
			
			write(mani, "1\n", 2); // mani version
			write(mani, "1\t1\t"); // modded and file version
			write(mani, filename, strlen(filename));
			write(mani, "\t", 1);
			write(mani, hash, strlen(hash)); // file's hashcode
			write(mani, "\n", 1);
			
			close(mani);
			return;

		}else{
			if(addManilist(filename, hash, mFront) == 0){
			
				updateMani(manipath, mFront, mani_version);
				return;
			}else{
				return;
			}
		}

	}else if(errno == ENOENT){ // project directory does NOT exist
		fprintf(stderr, "Error: Project directory does not exist on client side.\n");
		return;
	}else{ // something went wrong with opendir()
		fprintf(stderr, "Error: opendir() failed for the project's directory on the client side.\n");
		return;
	}
	
	return;
};

int main(int argc, char** argv){
	
	if(strcmp("add", argv[1]) == 0){
		if(argc != 4){
			fprintf(stderr, "Error: Insufficient number of arguments\n");
			return 1;
		}else{
			add(argv[2], argv[3]);
			return 0;
		}
	}

    if(argc == 4){
        if(strcmp("configure", argv[1]) == 0){
            config(argv[2], argv[3]);
            return 0;
        }
    }else{
        if(config_check() != 0) return 1;
    }

    struct data* connInfo = (struct data*) malloc(sizeof(struct data));
    build_info(connInfo);

    //create socket
    int network_socket;
	if((network_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Error: Could not create socket.\n");
		return 1;
	}
    
    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(connInfo->port));
   
    if(inet_pton(AF_INET, connInfo->ip, &server_address.sin_addr) <= 0){
		printf("Error: Invalid address. Address not supported.\n");
		return 1;
	}
    
    int connection_status = -1;
    
    while(connection_status == -1){
    	connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    	sleep(3);
    }
    
    // check for error with the connection
    if(connection_status == -1){
        printf("Error: Connection failed.\n\n");
    }

    // receive data from the server
    char server_response[256];
    char received[3];
    recv(network_socket, &server_response, sizeof(server_response), 0);
    
    if(argc == 3){
    
    	// CREATE command
        if(strcmp(argv[1], "create") == 0){
            send(network_socket, argv[1], sizeof(argv[1]), 0);
            recv(network_socket, &received, sizeof(received), 0);
            if(strcmp("OK", received) != 0){
            	fprintf(stderr, "Error: 'create' command failed to send.\n");
            	close(network_socket);
            	return 1;
          	}
            create(network_socket, argv[2], connInfo);
        }
        
        // DESTROY command
        else if(strcmp(argv[1], "destroy") == 0){
        	send(network_socket, argv[1], sizeof(argv[1]), 0);
            recv(network_socket, &received, sizeof(received), 0);
            if(strcmp("OK", received) != 0){
            	fprintf(stderr, "Error: 'destroy' command failed to send.\n");
            	close(network_socket);
            	return 1;
          	}
            destroy(network_socket, argv[2], connInfo);
        }
        
    }
    
    // close the socket
    close(network_socket);
    return 0;
}
