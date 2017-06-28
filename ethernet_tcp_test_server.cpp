#include <mraa.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string>
#include <sstream>
#include <vector>
#include <string.h>
#include <csignal>

const int PORT_NUMBER = 55056;
const int BUFFER_LENGTH = 256;

// format: "ar <analogPinNumber>\n"
const char* COMMAND_FORMAT_ANALOG_READ = "ar %d\n";
// format: "aw <analogPinNumber> <dacValue>\n" (depends on client dacValue)
const char* COMMAND_FORMAT_ANALOG_WRITE = "aw %d %d\n";
// format: "dr <digitalPinNumber>\n"
const char* COMMAND_FORMAT_DIGITAL_READ = "dr %d\n";
// format: "dw <digitalPinNumber> <digitalValue>\n"
const char* COMMAND_FORMAT_DIGITAL_WRITE = "dw %d %d\n";

void error(const char *);
int digitalReadCommand(char*, int);
int digitalWriteCommand(char*, int, int);
int analogReadCommand(char*, int);
int analogWriteCommand(char*, int, int);
void parseReply(char*, int);

bool stopped = false;

// Program close signal handler
void procSigIntHandler(int s) {
	stopped = true;
}

// Define input elements, which values are sent to the Arduino board
mraa::Gpio* button = NULL;
mraa::Aio* potentiometer = NULL;


int main(void) {

	// Initialize input elements
	button = new mraa::Gpio(0, true, false);
	button->dir(mraa::DIR_IN);
	potentiometer = new mraa::Aio(0);

	// Print TCP port number to console
	std::cout << "Hello, TCP! (" << PORT_NUMBER << ")" << std::endl;

	// Listen for program termination, so it can close sockets properly
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = procSigIntHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	// Create I/O char buffer
	char buffer[BUFFER_LENGTH];

	// Open TCP connection socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("ERROR opening socket");

	// Initialize server socket
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT_NUMBER);

	// Initialize client socket
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	// Bind server socket and start listening for clients
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");
	listen(sockfd, 5);

	// Wait until a client is connected
	int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) error("ERROR on accept");

	int n = BUFFER_LENGTH;

	int length;
	int buttonValue;
	int potValue;
	memset(buffer, 0, BUFFER_LENGTH);
	bool error = false;

	// Keep transmitting input values until program is terminated or client is disconnected
	while (!stopped) {

		if (error == true) {
			// Wait until any client is connected again
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			error = false;
		}

		// Read button value
		buttonValue = button->read();
		// Write output command into buffer
		length = digitalWriteCommand(buffer, 5, buttonValue);
		// Send digital write command to client (Arduino board)
		n = write(newsockfd, buffer, length);
		if (n < 0) {
			printf("ERROR writing to socket");
			error = true;
		}

		// Write input command into buffer
		length = analogReadCommand(buffer, 0);
		// Send analog read command to client (Arduino board)
		n = write(newsockfd, buffer, length);
		if (n < 0) {
			printf("ERROR writing to socket");
			error = true;
		}


		// Read potentiometer value
		potValue = (potentiometer->read() * 255) / 1024;
		// Write output command into buffer
		length = analogWriteCommand(buffer, 3, potValue);
		// Send analog write command
		n = write(newsockfd, buffer, length);
		if (n < 0) {
			printf("ERROR writing to socket");
			error = true;
		}

		// Read client command replies
		n = read(newsockfd, buffer, BUFFER_LENGTH - 1);

		// Parse input replies (if any)
		if (n > 0) {
			parseReply(buffer, n);
			memset(buffer, 0, BUFFER_LENGTH);
		}

		if (n < 0) {
			printf("ERROR reading from socket");
			error = true;
		}

		usleep(250 * 1000); // 2 seconds
	}

	std::cout << "Program terminated, closing sockets..." << std::endl;
	close(newsockfd);
	close(sockfd);

	return 0;
}


// Terminate program in case of errors
void error(const char *msg) {
    perror(msg);
    exit(1);
}


// Split received to read command arguments
std::vector<std::string> split(std::string str, std::string sep) {
    char* cstr = const_cast<char*>(str.c_str());
    char* current;
    std::vector<std::string> arr;
    current = strtok(cstr, sep.c_str());
    while (current != NULL) {
        arr.push_back(current);
        current = strtok(NULL, sep.c_str());
    }

    return arr;
}


// Create analog read command
int analogReadCommand(char* buffer, int pinNumber) {
	return sprintf(buffer, COMMAND_FORMAT_ANALOG_READ, pinNumber);
}


// Create analog write command
int analogWriteCommand(char* buffer, int pinNumber, int dacValue) {
	return sprintf(buffer, COMMAND_FORMAT_ANALOG_WRITE, pinNumber, dacValue);
}


// Create digital read command
int digitalReadCommand(char* buffer, int pinNumber) {
	return sprintf(buffer, COMMAND_FORMAT_DIGITAL_READ, pinNumber);
}


// Create digital write command
int digitalWriteCommand(char* buffer, int pinNumber, int digitalValue) {
	return sprintf(buffer, COMMAND_FORMAT_DIGITAL_WRITE, pinNumber, digitalValue);
}


// Split replies from each command
void parseReply(char* replies, int length) {

	printf("Received reply: %s\n", replies);
	char commandName[10];
	int pin;
	int value;
	int n;

	char* reply;
	reply = strtok(replies, "\n"); // Separate each received reply
	while (reply != NULL) {
		n = sscanf(reply, "%s %d %d", commandName, &pin, &value);

		if (n == 0) {
			std::cout << "Unknown reply: " << reply << std::endl;
			return;
		}

		if (strcmp(commandName, "ERROR:") == 0) {
			std::cout << "Client replied with error: " << reply << std::endl;
		} else if (strcmp(commandName, "dr") == 0) {
			// Client replied with digital reading result
			std::cout << "Digital pin value: " << pin << "-" << value << std::endl;
		} else if (strcmp(commandName, "dw") == 0) {
			// Command was successful, nothing to do
		} else if (strcmp(commandName, "ar") == 0) {
			// Client replied with analog reading result
			std::cout << "Analog pin value: " << pin << "-" << value << std::endl;
		} else if (strcmp(commandName, "aw") == 0) {
			// Command was successful, nothing to do
		} else {
			std::cout << "ERROR: Unknown reply: " << reply << std::endl;
		}

		reply = strtok(NULL, "\n");
	}
}

