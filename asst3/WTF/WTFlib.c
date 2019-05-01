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

/*
./WTF configure <IP> <port>
The configure command will save the IP address and port of the server for use by later commands. This
command will not attempt a connection to the server, but insteads saves the IP and port number so that they are
not needed as parameters for all other commands. The IP and port can be either saved out to a ./.configure file or
an environment variable by using extern. All commands that need to communicate with the server should first
try to get the IP address and port from either the ./.configure file or environment variable and must fail ifconfigure wasn’t run before they were called. All other commands must also fail if a connection to the server
cannot be established.*/
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
};

/*
./WTF create <project name>
The create command will fail if the project name already exists on the server or the client can not communicate
with the server. Otherwise, the server will create a project folder with the given name, initialize a .Manifest for it
and send it to the client. The client will set up a local version of the project folder in its current directory and
should place the .Manifest the server sent in it.*/
void create(int network, char* proj, struct data* connInfo){
    char response[256];
    char recvd[3];
    char* path;
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
    	path = (char*) malloc((sizeof(proj) + 32) * sizeof(char));
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

/*
./WTF destroy <project name>
The destroy command will fail if the project name doesn’t exist on the server or the 
client can not communicate with it. On receiving a destroy command the server should 
lock the repository, expire any pending commits, delete all files and subdirectories 
under the project and send back a success message.*/
void destroy(int network, char* proj, struct data* connInfo){
    char response[256];
    char recvd[3];
    recvd[0] = '\0';
    response[0] = '\0';
	
    send(network, proj, sizeof(proj), 0);
    recv(network, &recvd, sizeof(recvd), 0);
    
    if(strcmp(recvd, "OK") != 0){
    	printf("Error: Project name was not successfully sent to server.\n");
    	return;
    }
    
    recv(network, &response, sizeof(response), 0);
    
    if(strcmp(response, "success") == 0){
    	printf("Successfully destroyed contents of project.\n");
    }else{
    	fprintf(stderr, "%s\n", response);
    }
    
	return;
};

/* Frees manilist linked list */
void freeMani(struct manilist* front){
	struct manilist* ptr = front;
	struct manilist* prev;
	
	while(ptr != NULL){
		prev = ptr;
		ptr = ptr->next;
		free(prev);
	}

	return;
};

/* Updates the .Manifest file for the project using data from a manilist linked list */
void updateMani(char* manipath, struct manilist** front, int mani_version){
	struct manilist* ptr = *front;
	
	execl("rm", manipath);
	execl("touch", manipath);
	
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
		
		write(mani, ptr->filename, strlen(ptr->filename));
		write(mani, "\t", 1);
		
		write(mani, ptr->hash, strlen(ptr->hash));
		write(mani, "\n", 1);
		
		ptr = ptr->next;
	}
	
	close(mani);
	return;
};

/* Checks if file was in the manifest and adds to end of list of its not. */
int addManilist(char* filename, char* hash, struct manilist** front){
	struct manilist* ptr = *front;
	struct manilist* prev = NULL;
	
	while(ptr != NULL){
		if(strcmp(ptr->filename, filename) == 0){ // file is in manilist
			
			if(strcmp(ptr->hash, hash) == 0){ // file is already in mani and updated
				return 1;
				
			}else{ // file is in mani, but needs to be updated
				strcpy(ptr->hash, hash);
				ptr->version = ptr->version + 1;
				ptr->modded[0] = '1';
			}
			
			return 0;
		}else{
			prev = ptr;
			ptr = ptr->next;
		}
	}

	ptr = (struct manilist*) malloc(sizeof(struct manilist));
	ptr->modded[0] = '1';
	ptr->version = 1;
	strcpy(ptr->filename, filename);
	strcpy(ptr->hash, hash);
	
	if(prev == NULL){
		*front = ptr;
	}else{
		prev->next = ptr;
	}
	
	return 0;
};


int buildManilist(char* path, struct manilist** node){
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

/* Reads through the filename and gets the hash function of the file */
unsigned char* fileHash(char* filename){
	unsigned char* hash;
	char c[1];
	int size = 0;
	int f, i;
	
	f = open(filename, O_RDWR, 00666);
	size = lseek(f, 0, SEEK_END);
	
	char data[size];
	
	lseek(f, 0, SEEK_SET);
	
	for(i = 0; i < size; ++i){
		read(f, c, 1);
		data[i] = c[0];
	}
	data[i] = '\0';
	close(f);
	
	hash = SHA256(data, strlen(data), 0);
	
	return hash;
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
	int mani, f, i;
	int hashlen = 0;
	int mani_version = 0;
	char* manipath;
	int fileSize = 0;
	unsigned char* h;
	struct manilist* mFront = NULL;
	
	if(projDir){ // project directory exists

		// checks if filename is valid
		if(!S_ISREG(fileInfo.st_mode)){
			fprintf(stderr, "Error: Invalid filename.\n");
			return;
		}
		
		// build the path for the .Manifest
		manipath = (char*) malloc((sizeof(proj) + 20) * sizeof(char));
		strcpy(manipath, proj);		
		strcat(manipath, "/.Manifest");

		// get file's hash
		h = fileHash(filename);
		
		hashlen = strlen(h);
		char hash[hashlen+1];
		
		// copy hash unsigned chars into a string 
		for(i = 0; i < strlen(h); ++i){
			sprintf(hash+i, "%02x", h[i]);
		}

		// build a linked list of all the files in the .Manifest
		mani_version = buildManilist(manipath, &mFront);

		if(mani_version == 0){ // .Manifest version is 0 and file is first file to be added
			
			mani = open(manipath, O_RDWR, 00666);
			write(mani, "1\n", 2); // mani version
			write(mani, "1\t1\t", 4); // modded and file version
			write(mani, filename, strlen(filename));
			write(mani, "\t", 1);
			write(mani, hash, strlen(hash)); // file's hashcode
			write(mani, "\n", 1);
			
			close(mani);
			return;

		}else if(mani_version == -1){
			fprintf(stderr, "Error: .Manifest is not built properly.\n");
		}else{
			if(addManilist(filename, hash, &mFront) == 0){
			
				updateMani(manipath, &mFront, mani_version);
				freeMani(mFront);
				return;
			}else{
			
				freeMani(mFront);
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

/* Checks if file was in the manifest. Removes it from list. */
int removeManilist(char* filename, struct manilist** front){
	struct manilist* ptr = *front;
	struct manilist* prev = NULL;
	
	while(ptr != NULL){
		if(strcmp(ptr->filename, filename) == 0){ // file is in manilist
			
			if(prev == NULL){
				prev = ptr;
				ptr = ptr->next;
				free(ptr);
			}else if(ptr->next == NULL){
				prev->next = NULL;
				free(ptr);
			}else{
				prev->next = ptr->next;
				free(ptr);
			}
			return 0;
		}else{
			prev = ptr;
			ptr = ptr->next;
		}
	}
	
	return 1;
};

/* ./WTF remove <project name> <filename>
The remove command will fail if the project does not exist on the client.
The client will remove the entry for the given file from its own .Manifest.
(It is not required, but it may speed things up/make things easier for you
if you add a code in the .Manifest to signify that this file was removed 
locally and the server hasn't seen it yet) */
void removeFile(char* proj, char* filename){
	struct stat fileInfo;
	DIR* projDir = opendir(proj);
	stat(filename, &fileInfo);
	int mani, f, i;
	int hashlen = 0;
	int mani_version = 0;
	char* manipath;
	int fileSize = 0;
	unsigned char* h;
	struct manilist* mFront = NULL;
	
	if(projDir){ // project directory exists

		// checks if filename is valid
		if(!S_ISREG(fileInfo.st_mode)){
			return;
		}
		
		// build the path for the .Manifest
		manipath = (char*) malloc((sizeof(proj) + 20) * sizeof(char));
		strcpy(manipath, proj);		
		strcat(manipath, "/.Manifest");

		// build a linked list of all the files in the .Manifest
		mani_version = buildManilist(manipath, &mFront);

		if(mani_version == 0){ // .Manifest version is 0 so nothing to remove

			return;

		}else if(mani_version == -1){
		
			fprintf(stderr, "Error: .Manifest is not built properly.\n");
			
		}else{
			if(removeManilist(filename, &mFront) == 0){
			
				updateMani(manipath, &mFront, mani_version);
				freeMani(mFront);
				return;
			}else{
			
				freeMani(mFront);
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













