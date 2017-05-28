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

// format: "ar <analogicPinNumber>\n"
const char* COMMAND_FORMAT_ANALOGIC_READ = "ar %d\n";
// format: "aw <analogicPinNumber> <dacValue>\n" (dacValue must be 0..255)
const char* COMMAND_FORMAT_ANALOGIC_WRITE = "aw %d %d\n";
// format: "dr <digitalPinNumber>\n"
const char* COMMAND_FORMAT_DIGITAL_READ = "dr %d\n";
// format: "dw <digitalPinNumber> <digitalValue>\n"
const char* COMMAND_FORMAT_DIGITAL_WRITE = "dw %d %d\n";

void error(const char *);
int digitalReadCommand(char*, int);
int digitalWriteCommand(char*, int, int);
int analogicReadCommand(char*, int);
int analogicWriteCommand(char*, int, int);
void parseReply(char*, int);

bool stopped = false;

void procSigIntHandler(int s){
           printf("Caught signal %d\n", s); //TODO: DELETE
           stopped = true;
}

mraa::Gpio* button = NULL;
mraa::Aio* potentiometer = NULL;

int main(void) {

	button = new mraa::Gpio(0, true, false);
	button->dir(mraa::DIR_IN);
	potentiometer = new mraa::Aio(0);

	std::cout << "Hello, TCP! (" << PORT_NUMBER << ")" << std::endl;

	// Listen for program termination, so it can close sockets properly
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = procSigIntHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	char buffer[BUFFER_LENGTH];

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("ERROR opening socket");

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT_NUMBER);

	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");
	listen(sockfd, 5);
	int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) error("ERROR on accept");

	int n = BUFFER_LENGTH;

	int i = 0;
	int length;
	int buttonValue;
	int potValue;
	memset(buffer, 0, BUFFER_LENGTH);
	bool error = false;
	while (!stopped) {

		if (error == true) {
			// Keep looking for clients
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			error = false;
		}
//		length = digitalWriteCommand(buffer, 4, i%2 == 0 ? 1 : 0); // Toggle LED0 = ON/OFF
//		n = write(newsockfd, buffer, length);
//		if (n < 0) error("ERROR writing to socket");

		buttonValue = button->read();
		printf("%d", buttonValue);
		length = digitalWriteCommand(buffer, 5, buttonValue);
		n = write(newsockfd, buffer, length);
		if (n < 0) {
			printf("ERROR writing to socket");
			error = true;
		}

		length = analogicReadCommand(buffer, 0);
		n = write(newsockfd, buffer, length);
		if (n < 0) {
			printf("ERROR writing to socket");
			error = true;
		}

		potValue = (potentiometer->read() * 255) / 1024;
		length = analogicWriteCommand(buffer, 3, potValue);
		n = write(newsockfd, buffer, length);
		if (n < 0) {
			printf("ERROR writing to socket");
			error = true;
		}

		std::cout << "Command sent" << std::endl;
		n = read(newsockfd, buffer, BUFFER_LENGTH - 1);

		if (n > 0) {
			parseReply(buffer, n);
			memset(buffer, 0, BUFFER_LENGTH);
		}
		if (n < 0) {
			printf("ERROR reading from socket");
			error = true;
		}

		i++;

		usleep(250 * 1000); // 2 seconds
	}

	std::cout << "Program terminated, closing sockets..." << std::endl;
	close(newsockfd);
	close(sockfd);

	return 0;
}


void error(const char *msg) {
    perror(msg);
    exit(1);
}


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


int analogicReadCommand(char* buffer, int pinNumber) {
	return sprintf(buffer, COMMAND_FORMAT_ANALOGIC_READ, pinNumber);
}


int analogicWriteCommand(char* buffer, int pinNumber, int dacValue) {
	return sprintf(buffer, COMMAND_FORMAT_ANALOGIC_WRITE, pinNumber, dacValue);
}


int digitalReadCommand(char* buffer, int pinNumber) {
	return sprintf(buffer, COMMAND_FORMAT_DIGITAL_READ, pinNumber);
}


int digitalWriteCommand(char* buffer, int pinNumber, int digitalValue) {
	return sprintf(buffer, COMMAND_FORMAT_DIGITAL_WRITE, pinNumber, digitalValue);
}


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
			std::cout << "Digital pin value: " << pin << "-" << value << std::endl;

		} else if (strcmp(commandName, "dw") == 0) {
			// Command was successful, nothing to do
			printf("PIN: %d STATE: %d\n", pin, value);
		} else if (strcmp(commandName, "ar") == 0) {
			std::cout << "Analog pin value: " << pin << "-" << value << std::endl;
		} else if (strcmp(commandName, "aw") == 0) {
			// Command was successful, nothing to do
		} else {
			std::cout << "ERROR: Unknown reply: " << reply << std::endl;
		}

		reply = strtok(NULL, "\n");
	}
}

