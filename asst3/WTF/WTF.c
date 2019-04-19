#include "WTF.h"

/*
CLIENT COMMANDS
configure | checkout | update | upgrade
commit | push | create | create | destroy
add | remove | currentversion | history
rollback
*/

int main(int argc, char** argv){

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
		printf("Error: Could not create socket.\n");
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

    // recieve data from the server
    char server_response[256];
    char received[3];
    recv(network_socket, &server_response, sizeof(server_response), 0);
    // print out the server's response
    printf("The server sent the data: %s\n", server_response);
    
    if(argc == 3){
        if(strcmp(argv[1], "create") == 0){
            send(network_socket, argv[1], sizeof(argv[1]), 0);
            recv(network_socket, &received, sizeof(received), 0);
            printf("recieved message: |%s|\n", received);
            if(strcmp("OK", received) != 0){
            	fprintf(stderr, "Error: 'create' command did not send successfully.\n");
            	close(network_socket);
            	return 1;
          	}
            create(network_socket, argv[2], connInfo);
        }
    }
    
    // close the socket
    close(network_socket);
    return 0;
}
