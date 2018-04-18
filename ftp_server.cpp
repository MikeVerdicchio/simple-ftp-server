/* Mike Verdicchio
 * Simple FTP Server
 * ftp_server.h
 *
 * This program is an implementation of an
 * FTP Server to ist, store, and retrieve files
 * from the local file system. It uses the current
 * working directory of the ftp_server program as the
 * file system directory.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftp_server.h"
using namespace std;

int main (int argc, char** argv) {
	char write_buffer[BUFFER_SIZE];	// Buffer that is used for sending to client
	char read_buffer[BUFFER_SIZE];	// Buffer that is used for reading from client
	char byte_buffer[FILE_SIZE];	// Buffer that is used on data socket for RETR/STOR
	int connection_port, data_port;	// Ports for control and data sockets
	int num_bytes;					// Integer to represent number of bytes written/read
	char* temp_token;				// char to hold a single token at a time
  	vector<char*> tokens;			// Vector of tokens in the input (separated by spaces)

	// Create file descriptor for connection and data sockets
	struct sockaddr_in connection_socket_addr;
	struct sockaddr_in data_socket_addr;

	// Ensure that a port number is passed as an argument
	if (argc != 2) {
		cerr << "Please enter the socket number as an argument." << endl;
		exit(EXIT_FAILURE);
	} else {
		connection_port = atoi(argv[1]);
	}

	// Create control-line socket and initialize data socket to -1
	int connection_socket = CreateSocketOnLocalhost(connection_socket_addr, connection_port, true);
	if(connection_socket == -1) exit(EXIT_FAILURE);
	int data_socket = -1;

	// Infinite while loop to keep programming running as daemon
	while(1) {
		// Create connection file descriptor by accepting socket
		int connection = accept(connection_socket, NULL, NULL);
		if (connection < 0) {
			perror("Accept failed.");
			exit(EXIT_FAILURE);
		}

		// Respond with 220 response - successful connection
		sprintf(write_buffer, RESPONSE_220);
		num_bytes = write(connection, write_buffer, strlen(write_buffer));
		
		// While loop that continue as long as a client is connected
		while(1) {
			// Read from client byte-by-byte to check for newlines
            for(int i = 0; i < BUFFER_SIZE; i++) {
                num_bytes = read(connection, &read_buffer[i], 1);
                if (num_bytes <= 0)  break;
                if (read_buffer[i] == '\n') {
                    read_buffer[i] = '\0';
                    break;
                }
            }
            if (num_bytes <= 0) break;

			// For debugging - output commandt that the client sent the server
            //cout << "The client sent: " <<  read_buffer << endl;

			// NOOP command - do nothing but send 200 response
			if(strncmp(read_buffer, "NOOP", 4) == 0) {
				sprintf(write_buffer, RESPONSE_200);
				num_bytes = write(connection, write_buffer, strlen(write_buffer));
			}

			// STRU command - only implements FILE (F). Send 200 if F, 504 otherwise.
			else if(strncmp(read_buffer, "STRU", 4) == 0) {
				char type[2];
				memset(&type[0], 0, sizeof(type));
				sscanf(read_buffer, "STRU %s", type);
				if(type[0] == 'F') {						// if F, send 200 response
					sprintf(write_buffer, RESPONSE_200);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
				} else {									// otherwise, send 504 response
					sprintf(write_buffer, RESPONSE_504);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
				}
			}

			// MODE command - only implements STREAM (S). Send 200 if S, 504 otherwise
			else if(strncmp(read_buffer, "MODE", 4) == 0) {
				char type[2];
				memset(&type[0], 0, sizeof(type));
				sscanf(read_buffer, "MODE %s", type);
				if(type[0] == 'S') {						// if S, send 200 response
					sprintf(write_buffer, RESPONSE_200);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
				} else {									// otherwise, send 504 response
					sprintf(write_buffer, RESPONSE_504);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
				}
			}	

			// TYPE command - sets the type to ascii (not implemented) or binary
			else if(strncmp(read_buffer, "TYPE", 4) == 0) {
				char type[2];
				memset(&type[0], 0, sizeof(type));
				sscanf(read_buffer, "TYPE %s", type);
				if(type[0] == 'I') {						// if I, send 200, set binary to true
					binary = true;
					sprintf(write_buffer, RESPONSE_200);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
				} else {									// otherwise, send 504, set binary to false
					binary = false;
					sprintf(write_buffer, RESPONSE_504);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
				}
			}

			// RETR command - downloads a file from the server to the client
			// Only runs if binary TYPE is set, otherwise a 451 response is sent
			else if(strncmp(read_buffer, "RETR", 4) == 0) {
				if(binary == true) {
					char file[50];
					memset(&file[0], 0, sizeof(file));
					sscanf(read_buffer, "RETR %s", file);

					// Check to see if file can be opened. Throw 450 if not. 
					if(fopen(file, "rb") == NULL) {
						sprintf(write_buffer, RESPONSE_450);
						num_bytes = write(connection, write_buffer, strlen(write_buffer));
					} else {
						// Send 150 response to start transfer
						sprintf(write_buffer, RESPONSE_150);
						num_bytes = write(connection, write_buffer, strlen(write_buffer));

						// Open file on server and read FILE_SIZE bytes and send to client over data socket.
						// Continuously do this until 0 bytes are read from the file.
						ifstream tempFile(file, ios::binary);
						if(tempFile.is_open()) {
							while(1) {
								tempFile.read(byte_buffer, FILE_SIZE);
								if(tempFile.gcount() == 0) break;
								num_bytes = write(data_socket, byte_buffer, tempFile.gcount());
							}
						}
						
						// Send 226 response to end transfer
						sprintf(write_buffer, RESPONSE_226);
						num_bytes = write(connection, write_buffer, strlen(write_buffer));
					}
				} else {
					// Send 451 response if TYPE is not binary
					sprintf(write_buffer, RESPONSE_451);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
				}
				close(data_socket);
			}

			// STOR command - uploads a file from the client to the server
			// Only runs if binary TYPE is set, otherwise a 451 response is sent
			else if(strncmp(read_buffer, "STOR", 4) == 0) {
				if(binary == true) {
					char file[50];
					memset(&file[0], 0, sizeof(file));
					sscanf(read_buffer, "STOR %s", file);
					
					// Check to see if file can be opened. Throw 450 if not. 
					if(fopen(file, "w") == NULL) {
						sprintf(write_buffer, RESPONSE_450);
						num_bytes = write(connection, write_buffer, strlen(write_buffer));
					} else {
						// Send 150 response to start transfer
						sprintf(write_buffer, RESPONSE_150);
						num_bytes = write(connection, write_buffer, strlen(write_buffer));

						// Open file on server and read FILE_SIZE bytes from client over data socket.
						// Continuously do this until 0 bytes are read from the client.
						ofstream tempFile(file, ios::binary);
						if(tempFile.is_open()) {
							while(1) {
								num_bytes = read(data_socket, byte_buffer, FILE_SIZE);
								if(num_bytes == 0) break;
								tempFile.write(byte_buffer, num_bytes);
							}
						}
						tempFile.close();

						// Send 226 response to end transfer
						sprintf(write_buffer, RESPONSE_226);
						num_bytes = write(connection, write_buffer, strlen(write_buffer));
					}
				} else {
					sprintf(write_buffer, RESPONSE_451);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
				}
				close(data_socket);
			}

			// PORT command - sets port to create data connection socket
			else if(strncmp(read_buffer, "PORT", 4) == 0) {
				unsigned char port[2];
				memset(&port[0], 0, sizeof(port));
				sscanf(read_buffer, "PORT 127,0,0,1,%d,%d", (int*)&port[0], (int*)&port[1]);

				// Calculate the data port, which is 256*port[0] + port[1]
				data_port = port[0] * 256 + port[1];

				// Create the socket on localhost at that port 
				data_socket = CreateSocketOnLocalhost(data_socket_addr, data_port, false);
				if(data_socket == -1) exit(EXIT_FAILURE);

				// Connect to the data socket and send a 200 response if successful, 425 if not
				if(connect(data_socket, (struct sockaddr *)&data_socket_addr, sizeof(data_socket_addr)) != 0) {
					perror("Error connecting to socket");
					sprintf(write_buffer, RESPONSE_425);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
					close(data_socket);
				} else {
					sprintf(write_buffer, RESPONSE_200);
					num_bytes = write(connection, write_buffer, strlen(write_buffer));
				}
			}

			// LIST commands - lists the files in the directory
			else if(strncmp(read_buffer, "LIST", 4) == 0) {
				char file[50];
				memset(&file[0], 0, sizeof(file));
				sscanf(read_buffer, "LIST %s", file);
				// These lines create a system call to ls -l (with a directory if supplied)
				// and writes it to a temp file. The file can then be read and formatted
				string command;
				if(string(file) == "") command = "ls -l > .ls.txt";
				else command = "ls -l " + string(file) + " > .ls.txt";
				system(command.c_str());

				// Send a 150 response to start the transfer
				sprintf(write_buffer, RESPONSE_150);
				num_bytes = write(connection, write_buffer, strlen(write_buffer));

				// Open the temp file and read it line by line
				string line;
				ifstream tempFile(".ls.txt");
				if(tempFile.is_open()) {
					char* input;
					while(getline(tempFile, line)) {
						input = new char[line.length() + 1];
						strcpy(input, line.c_str());

						// Break input into tokens separated by spaces
						if(!tokens.empty()) tokens.clear();		// If the tokens array isn't empty, then clear it
						temp_token = strtok(input, " ");		// Find next token group (separated by spaces)
						while (temp_token != NULL) {
							tokens.push_back(temp_token);		// Add the token to the vector of tokens
							temp_token = strtok(NULL, " ");		// Find next token (separated by spaces)
						}

						// If line is a full line (not a count line), write it to the client
						// File/directory name [tab] size (in bytes)
						if(tokens.size() != 9) continue;
						line = string(tokens[8]) + string("\t") + string(tokens[4]) + string("\r\n");
						sprintf(write_buffer, line.c_str());
						num_bytes = write(data_socket, write_buffer, strlen(write_buffer));
					}
					tempFile.close();
				}

				// Remove the temp file, since it is no longer needed
				system("rm .ls.txt");

				// Send a 226 response to signal end of transfer
				sprintf(write_buffer, RESPONSE_226);
				num_bytes = write(connection, write_buffer, strlen(write_buffer));

				close(data_socket);
			}

			// QUIT command - closes the connection
			else if(strncmp(read_buffer, "QUIT", 4) == 0) {
				// Send a 221 response to signal closed connection
				sprintf(write_buffer, RESPONSE_221);
				num_bytes = write(connection, write_buffer, strlen(write_buffer));
				// Close the control socket
				close(connection);
			}
			
			// Respond with 202 response - command not implemented
			else {
				sprintf(write_buffer, RESPONSE_202);
				num_bytes = write(connection, write_buffer, strlen(write_buffer));
			}

		}

		close(connection);
	}

	close(connection_socket);
	
	return 0;
}

// This function creates a socket on localhost (127.0.0.1) with
// a given port with the passed in sockaddr struct.
int CreateSocketOnLocalhost(sockaddr_in &sa, int port, bool control) {
	// Create socket file descriptor
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Exit program if socket creation fails
	if (SocketFD == -1) {
		perror("Error creating socket.");
		return -1;
	}

	// Set SO_REUSEADDR to allow for reusing of ports
	int enable = 1;
	setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

	// Fill memory for socket address struct 
	memset(&sa, 0, sizeof(sa));

	// Set up IP address for localhost and port for given argument
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");

	// If the connection is the control connection, you must bind() and listen() to it
	// If the connection is the data connection, 
	if(control) {
		// Bind() to socket
		if(bind(SocketFD, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
			perror("Bind to port failed.");
			close(SocketFD);
			return -1;	
		}
		
		// Listen() on socket
		if (listen(SocketFD, 10) == -1) {
			perror("Listen failed.");
			close(SocketFD);
			return -1;
		}
	}

	// Return Socket file descriptor to be used
	return SocketFD;
}