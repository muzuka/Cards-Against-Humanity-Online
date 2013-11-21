/*
 *  The client acts as the player in the game.
 *  
 *
 *
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "Card.h"
#include "Player.h"

using namespace std;

bool isDone = false;

Player self;
Player judge;

void openMenu();
char* composeSENDMessage(char, Card);
char* composeNOTIFYMessage(char, char*);

int main(int argc, char* argv[]) {
	
	int sock;
	struct sockaddr_in serverAddr;
	
	if (argc != 3) {
		printf("Usage: %s <server IP> <Server Port>\n", argv[0]);
		exit(1);
	}
	
	string name;
	printf("Welcome to Cards Against Humanity Online!\n");
	printf("Experience all of the hilarious and bad humour of the real thing.\n");
	printf("What do you want as a username:\n");
	cin >> name;
	self.setName(name);
	printf("Please wait until you have connected.\n");
	
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("socket() failed\n");
		exit(1);
	}
	self.setSocket(sock);
	
	int yes = 1;
	if (setsockopt(self.getSocket(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
		printf("setsockopt() failed\n");
		exit(1);
	}
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	
	printf("Connecting to server.\n");
	if (connect(self.getSocket(), (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
		printf("connect() failed\n");
		exit(1);
	}
	printf("Connected to Server successfully.\n");
	bool isReady = false;
	while (!isReady) {
		send(self.getSocket(), self.getName().c_str(), self.getName().length(), 0);
		
		
	}
	
	while(!isDone) {
		
	}
}

// composes a message to send, made of the card and source
/*
 * messag types: 'p' for post, 'n' for answer, 'd' for add
 */
char* composeSENDMessage(char type, Card cardToSend) {
	if(type == 'p') {
		return strcat((char*)"POST Server\n", cardToSend.content.c_str());
	}
	else if(type == 'n') {
		return strcat((char*)"ANSWER Server\n", cardToSend.content.c_str());
	}
	else if(type == 'd') {
		return strcat((char*)"ADD Server\n", cardToSend.content.c_str());
	}
	else {
		return NULL;
	}
	
}

// Composes a message to notify, made of the info to send
/*
 *
 */
char* composeNOTIFYMessage(char purpose, char* playerName) {
	if(purpose == 'j') {
		return strcat((char*)"CP: ", playerName);
	}
	else if(purpose == 'w') {
		return strcat((char*)"winner: ", playerName);
	}
	else {
		return NULL;
	}
	
}