#include "WTFserver.h"

void sCreate(int client){
	char proj[256];
	char response[256];
	char path[512] = "./repository";
	
	proj[0] = '\0';
	response[0] = '\0';
	
	DIR* repos = opendir("./repository");
	
	recv(client, &proj, sizeof(proj), 0);
	if(proj[0] == '\0'){
		fprintf(stderr, "Error: Did not receive project name from client\n");
		return;
	}
	printf("Project name received: %s\n", proj);
	
	if(repos){
		printf("Ready to enter /repository folder\n");
	}else if(errno == ENONET){
		printf("Repository folder does not exist. Creating now\n");
		mkdir(path, ACCESSPERMS); // makes repos dir
		printf("Repository folder made\n");
		strcat(path, "/");
		strcat(path, proj);
		mkdir(path, ACCESSPERMS); //makes proj dir
		strcpy(response, "Project created successfully.");
		send(client, response, sizeof(response), 0);
	}else{
		fprintf(stderr, "Error: opendir(\"./repository\") failed.\n");
		strcpy(response, "opendir(\"./repositry\") failed");
		send(client, response, sizeof(response), 0);
		return;
	}
	
	return;
};
