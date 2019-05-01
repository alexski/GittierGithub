#include "WTF.h"

/*  CLIENT COMMANDS
configure | checkout | update | upgrade |
commit | push | create | destroy | add |
remove | currentversion | history |
rollback  */

/* ./WTF currentversion <project name>
The currentversion command will request from the server the current state 
of a project from the server. This command does not require that the 
client has a copy of the project locally. The client should output a list 
of all files under the project name, along with their version number 
(i.e., number of updates). */
void currentversion(int network, char* proj, struct data* connInfo){
	char response[256];
    char recvd[3];
    char* path;
    recvd[0] = '\0';
    response[0] = '\0';
    int done = 0;
    
    send(network, proj, sizeof(proj), 0);
    recv(network, &recvd, sizeof(recvd), 0);
    
    if(strcmp(recvd, "OK") != 0){
    	printf("Error: Project name was not successfully sent to server.\n");
    	return;
    }
    
    recv(network, &response, sizeof(response), 0);
    
	if(strcmp(response, "success") == 0){
		recv(network, &response, sizeof(response), 0);
		
		if(strcmp(response, "none") == 0){
			return;
		}else if(strcmp(response, "full") == 0){ // there are contents to printf
			
			while(done != 1){
				recv(network, &response, sizeof(response), 0);
				
				if(strcmp(response, "done") == 0){
					done = 1;
				}else{
					printf("%s\n", response);
				}
			}
			
			return;
		}else{ // error
			fprintf(stderr, "%s\n", response);
			return;
		}
		
	}else{
		fprintf(stderr, "%s\n", response);
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
	}else if(strcmp("remove", argv[1]) == 0){
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
        
        else if(strcmp(argv[1], "currentversion") == 0){
        	send(network_socket, argv[1], sizeof(argv[1]), 0);
            recv(network_socket, &received, sizeof(received), 0);
            if(strcmp("OK", received) != 0){
            	fprintf(stderr, "Error: 'destroy' command failed to send.\n");
            	close(network_socket);
            	return 1;
          	}
          	currentversion(network_socket, argv[2], connInfo);
        }
        
    }
    
    // close the socket
    close(network_socket);
    return 0;
}
