#include "WTFserver.h"

void sCreate(int client){
	char proj[256];
	char response[256];
	char path[512] = "./repository";
	
	proj[0] = '\0';
	response[0] = '\0';
	
	DIR* repos = opendir("./repository");
	int mani;
	
	recv(client, &proj, sizeof(proj), 0);
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
		}else if(errno == ENOENT){
			mkdir(path, ACCESSPERMS); //makes proj dir
			strcpy(response, "Project directory created successfully.");
			
			
			// Initialize .Manifest file in project folder
			strcat(path, "/.Manifest");
			mani = open(path, O_RDWR | O_CREAT, 00666);
			write(mani, "0\n", 2);
  			close(mani);
			
			/*
			JUST NEED TO SEND FILE OVER TO CLIENT
			*/
			
			// send over what happened to client
			send(client, response, sizeof(response), 0);
			
		}else{ // something wrong happened with opendir()
			fprintf(stderr, "Error: opendir(\"%s\") failed.\n", path);
			strcpy(response, "Error: opendir(\"");
			strcat(response, path);
			strcat(response, "\") failed");
			send(client, response, sizeof(response), 0);
		}
		
	}else if(errno == ENOENT){ // repository folder does not exist
		
		// build the repos folder
		printf("Repository folder does not exist. Creating now\n");
		mkdir(path, ACCESSPERMS);
		printf("Repository folder made\n");
		
		strcat(path, "/");
		strcat(path, proj);
		
		// build the project folder
		mkdir(path, ACCESSPERMS);
		printf("Project folder created.");
		
		// Initialize .Manifest file in project folder
		strcat(path, "/.Manifest");
		mani = open(path, O_RDWR, 00666);
		write(mani, "0\n", 2);
		close(mani);

		
		/*
		JUST NEED TO SEND FILE OVER TO CLIENT
		*/		
		
		// send over what happened to client
		strcpy(response, "Project created successfully.");
		send(client, response, sizeof(response), 0);
		
	}else{ // something went wrong with opendir()
		fprintf(stderr, "Error: opendir(\"./repository\") failed.\n");
		strcpy(response, "opendir(\"./repositry\") failed");
		send(client, response, sizeof(response), 0);
	}
	
	// close repos dir
	closedir(repos);
	return;
};
