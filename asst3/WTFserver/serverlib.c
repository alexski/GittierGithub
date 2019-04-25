#include "WTFserver.h"

void sCreate(int client){
	char proj[256];
	char response[256];
	char sent[3] = "OK";
	char path[512] = "./repository";
	struct stat* manistat;
	int mani_size;
	char ms[10];
	
	proj[0] = '\0';
	response[0] = '\0';
	
	DIR* repos = opendir("./repository");
	int mani;
	
	recv(client, &proj, sizeof(proj), 0);
	send(client, sent, sizeof(sent), 0);
	
	if(proj[0] == '\0'){
		fprintf(stderr, "Error: Did not receive project name from client\n");
		return;
	}
	
	if(repos){ // if repository directory exists, go in and check if <project name> dir exists
		strcat(path, "/");
		strcat(path, proj);
		DIR* projDir = opendir(path);
		
		if(projDir){
			strcpy(response, "Error: Project already exists.");
		
			send(client, response, sizeof(response), 0);
			
		}else if(errno == ENOENT){ // proj dir does not exist therefore one must be created
			mkdir(path, ACCESSPERMS); //makes proj dir
			strcpy(response, "success");
			send(client, response, sizeof(response), 0);
			
			// build metadata file
			char* meta_path = (char*) malloc(sizeof(path) * sizeof(char));
			strcpy(meta_path, path);
			strcat(meta_path, "/.metadata");
			
			// write in the latest version of project
			int md = open(meta_path, O_RDWR | O_CREAT, 00666);
			write(md, "0", 1);
			close(md);
			free(meta_path);
			
			// build the /<version number> directory in the project directory
			strcat(path, "/0");
			mkdir(path, ACCESSPERMS);
			
			// Initialize .Manifest file in project folder
			strcat(path, "/.Manifest");
			mani = open(path, O_RDWR | O_CREAT, 00666);
			write(mani, "0\n", 2);
  			close(mani);
  					
			send(client, "0\n", 2, 0);
			
		}else{ // something wrong happened with opendir() in project dir
			fprintf(stderr, "Error: opendir() failed for project's directory on server side.\n");
			strcpy(response, "Error: opendir() failed for project's directory server side.");
			send(client, response, sizeof(response), 0);
		}
		
	}else if(errno == ENOENT){ // repository folder does not exist
		
		// build the repos folder
		mkdir(path, ACCESSPERMS);
		
		// build the project folder
		strcat(path, "/");
		strcat(path, proj);
		mkdir(path, ACCESSPERMS);
		
		strcpy(response, "success");
		send(client, response, sizeof(response), 0);
		
		// build metadata file
		char* meta_path = (char*) malloc(sizeof(path) * sizeof(char));
		strcpy(meta_path, path);
		strcat(meta_path, "/.metadata");
		
		// write in the latest version of project
		int md = open(meta_path, O_RDWR | O_CREAT, 00666);
		write(md, "0", 1);
		close(md);
		free(meta_path);
		
		// build the /<version number> directory in the project directory
		strcat(path, "/0");
		mkdir(path, ACCESSPERMS);
		
		// Initialize .Manifest file in project folder
		strcat(path, "/.Manifest");
		mani = open(path, O_RDWR | O_CREAT, 00666);
		write(mani, "0\n", 2);
		close(mani);
		
		send(client, "0\n", 2, 0);
		
	}else{ // something went wrong with opendir()
		fprintf(stderr, "Error: opendir() failed for repository directory on server side.\n");
		strcpy(response, "opendir() failed for repository directory on server side.");
		send(client, response, sizeof(response), 0);
	}
	
	// close repos dir
	closedir(repos);
	return;
};

// deletes all files and directories in given path
int clearDir(char* path){
	DIR* dir = opendir(path);
	int r = -1;
	
	if(dir){
		struct dirent* dirInfo;
		
		r = 0;
		
		while(!r && (dirInfo = readdir(dir))){
			int r2 = -1;
			char* item;
			int length;
			
			if(!strcmp(dirInfo->d_name, ".") || !strcmp(dirInfo->d_name, "..")){
				continue;
			}
			
			length = strlen(path) + strlen(dirInfo->d_name) + 2;
			item = (char*) malloc(length);
			
			if(item){
				struct stat itemstat;
				snprintf(item, length, "%s/%s", path, dirInfo->d_name);
				 
				if(!stat(item, &itemstat)){
					
					if(S_ISDIR(itemstat.st_mode)){ // if the item is directory, perform recursion
						r2 = clearDir(item);
					}else{
						r2 = unlink(item);
					}
					
				}
				
				free(item);
			}
			
			r = r2;
		}
		closedir(dir);
				
	}
	
	if(!r){
		r = rmdir(path);
	}
	
	return r;
};

/* STILL NEED TO FIGURE OUT HOW TO LOCK THE REPOSITORY FOR THIS COMMAND */
void sDestroy(int client){
	char proj[256];
	char response[256];
	char sent[3] = "OK";
	char path[512] = "./repository";
	
	proj[0] = '\0';
	response[0] = '\0';
	
	DIR* repos = opendir("./repository");
	
	recv(client, &proj, sizeof(proj), 0);
	send(client, sent, sizeof(sent), 0);
	
	if(proj[0] == '\0'){
		fprintf(stderr, "Error: Did not receive project name from client.\n");
		return;
	}
	
	if(repos){ // if repository directory exists, go in and check if <project name> dir exists
		strcat(path, "/");
		strcat(path, proj);
		DIR* projDir = opendir(path);
		
		if(projDir){
			// remove all files and directories
			closedir(projDir);
			
			clearDir(path);
			
			strcpy(response, "success");
			send(client, response, sizeof(response), 0);
			
		}else if(errno == ENOENT){ // proj dir does not exist
			strcpy(response, "Error: Project does not exist on server side.");
			send(client, response, sizeof(response), 0);
			
		}else{ // something wrong happened with opendir() in project dir
			fprintf(stderr, "Error: opendir() failed for project's directory on server side.\n");
			strcpy(response, "Error: opendir() failed for project's directory server side.");
			send(client, response, sizeof(response), 0);
		}
		
	}else if(errno == ENOENT){ // repository folder does not exist		
		strcpy(response, "Error: Project does not exist on server side.");
		send(client, response, sizeof(response), 0);
		
	}else{ // something went wrong with opendir()
		fprintf(stderr, "Error: opendir() failed for repository directory on server side.\n");
		strcpy(response, "opendir() failed for repository directory on server side.");
		send(client, response, sizeof(response), 0);
	}
	
	// close repos dir
	closedir(repos);
	return;
};
