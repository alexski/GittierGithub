#include "WTFserver.h"

// USE man socket IN TERMINAL FOR MORE INFORMATION

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
			}
		}else{
			fprintf(stderr, "Error: Message not received from client.\n");
		}
	
	close(client_socket);
	return 0;
}
