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
	printf("Project name received: %s\n", proj);
	
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
			
			// Initialize .Manifest file in project folder
			strcat(path, "/.Manifest");
			mani = open(path, O_RDWR | O_CREAT, 00666);
			write(mani, "0\n", 2);
  			close(mani);
  					
			send(client, "0\n", 2, 0);
  			
  			/*
			// get file info
			stat(path, manistat);
			mani_size = manistat->st_size;
			snprintf(ms, sizeof(ms), "%d", mani_size);
		
			// send client file size of .Manifest
			send(client, ms, sizeof(ms), 0);
	
			char data[mani_size];
		
			read(mani, data, sizeof(data));
			send(client, data, sizeof(data), 0);			
			*/
			
			/*
			JUST NEED TO SEND FILE OVER TO CLIENT
			*/
			
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
		
		// Initialize .Manifest file in project folder
		strcat(path, "/.Manifest");
		mani = open(path, O_RDWR | O_CREAT, 00666);
		write(mani, "0\n", 2);
		close(mani);
		
		send(client, "0\n", 2, 0);
		/*
		// get file info
		stat(path, manistat);
		mani_size = (int) manistat->st_size;
		snprintf(ms, sizeof(ms), "%d", mani_size);
		
		// send client file size of .Manifest
		send(client, ms, sizeof(ms), 0);
		
		char data[mani_size+1];
    
		read(mani, data, sizeof(data));
		send(client, data, sizeof(data), 0);		
		*/
		
	}else{ // something went wrong with opendir()
		fprintf(stderr, "Error: opendir() failed for repository directory on server side.\n");
		strcpy(response, "opendir() failed for repository directory on server side.");
		send(client, response, sizeof(response), 0);
	}
	
	// close repos dir
	closedir(repos);
	return;
};
