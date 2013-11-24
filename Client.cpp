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
#include <string>
#include <string.h>
#include <unistd.h>
#include "Card.h"
#include "Player.h"

using namespace std;

bool isDone = false;

Card blackCard;
Player self;
Player judge;
bool isJudge = false;
int bytesRecv = 0;
int bytesSent = 0;
char inBuffer[200];
char outBuffer[200];
vector<Card> answers;

void printHand(vector<Card>);
void printMessage(int);
void parseMessage(string);
string composeSENDMessage(char, Card);
string composeNOTIFYMessage(char, string);
int countChars(char*, char);
void splitString(char**, char*, const char*);

int main(int argc, char* argv[]) {
	
	int sock;
	struct sockaddr_in serverAddr;
	
	if (argc != 3) {
		printf("Usage: %s <server IP> <Server Port>\n", argv[0]);
		exit(1);
	}
	
	string name;
	printMessage(0);
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
	send(self.getSocket(), self.getName().c_str(), self.getName().length(), 0);
	while(!isReady) {

		bytesRecv = 0;
		while(bytesRecv <= 0) {
			bytesRecv = recv(self.getSocket(), (char*)&outBuffer, 1, 0);
		}
		if (outBuffer[0] == 'y') {
			break;
		}
		else if(outBuffer[0] == 'n'){
			printf("The name was already taken, please enter another:\n");
			cin >> name;
			self.setName(name);
		}
		
	}
	printf("The name was accepted!\n");
	
	for(int i = 0; i < 10; i++) {
		
		bytesRecv = 0;
		while(bytesRecv <= 0) {
			bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
		}
		parseMessage(string(inBuffer));
		printf("Got a card.\n");
	}
	
	if(self.getHand().empty()) {
		 printf("Hand is empty.\n");
	}
	else {
		printHand(self.getHand());
	}
	while(!isDone) {
		
		bytesRecv = 0;
		while(bytesRecv <= 0) {
			bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
		}
		parseMessage(string(inBuffer));
		
		bytesRecv = 0;
		while(bytesRecv <= 0) {
			bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
		}
		parseMessage(string(inBuffer));
		
		
		
		if(isJudge) {
			
		}
		else {
			
		}

	}
}

void printHand(vector<Card> hand) {
	for (int i = 0; i < hand.size(); i++) {
		printf("%s\n", hand[i].content.c_str());
	}
}

void printMessage(int message) {
	switch (message) {
		case 0:
			printf("Welcome to Cards Against Humanity Online!\n");
			printf("Experience all of the hilarious and bad humour of the real thing.\n");
			printf("What do you want as a username:\n");
			break;
		case 1:
			
			break;
		default:
			break;
	}
}

// Given a string, it parses and does the action required by the message.
void parseMessage(string message) {
	char* notifyMessage[2];
	char* messageDiv[2];
	char* getCommand[2];
	splitString(messageDiv, (char*)message.c_str(), "\n");
	splitString(getCommand, messageDiv[0], " ");
	
	if(strcmp((const char*)getCommand[0], "ADD") == 0) {
		//printf("Got an ADD message.\n");
		self.addCard(Card(string(messageDiv[1]), 'w', 0, self));
	}
	else if(strcmp((const char*)getCommand[0], "POST") == 0) {
		blackCard = Card(string(messageDiv[1]), 'b', countChars(messageDiv[1], '_'));
	}
	else if(strcmp((const char*)getCommand[0], "ANSWER") == 0) {
		answers.push_back(Card(string(messageDiv[1]), 'w', 0, Player(string(getCommand[1]), 0)));
	}
	else if(strcmp((const char*)getCommand[0], "NOTIFY") == 0) {
		splitString(notifyMessage, messageDiv[1], " ");
		if(strcmp((const char*)notifyMessage[0], "CP:") == 0) {
			if (strcmp(self.getName().c_str(), (const char*)notifyMessage[1]) == 0) {
				isJudge = !isJudge;
				judge = self;
			}
			else {
				judge.setName(string(notifyMessage[1]));
			}
		}
		else if(strcmp((const char*)notifyMessage[0], "winner:") == 0) {
			
		}
	}
}

// composes a message to send, made of the card and source
/*
 * messag types: 'p' for post, 'n' for answer, 'd' for add
 */
string composeSENDMessage(char type, Card cardToSend) {
	char* t;
	if(type == 'p') {
		/*t = strcat((char*)"POST ", self.getName().c_str());
		t = strcat(t, "\n");
		return strcat(t, cardToSend.content.c_str());*/
		return "POST " + self.getName() + "\n" + cardToSend.content;
	}
	else if(type == 'n') {
		/*t = strcat((char*)"ANSWER ", self.getName().c_str());
		t = strcat(t, "\n");
		return strcat(t, cardToSend.content.c_str());*/
		return "ANSWER " + self.getName() + "\n" + cardToSend.content;
	}
	else if(type == 'd') {
		/*t = strcat((char*)"ADD ", self.getName().c_str());
		t = strcat(t, "\n");
		return strcat(t, cardToSend.content.c_str());*/
		return "ADD " + self.getName() + "\n" + cardToSend.content;
	}
	else {
		return "";
	}
	
}

// Composes a message to notify, made of the info to send
/*
 *
 */
string composeNOTIFYMessage(char purpose, string playerName) {
	if(purpose == 'j') {
		//return strcat((char*)"CP: ", playerName);
		return "CP: " + playerName;
	}
	else if(purpose == 'w') {
		//return strcat((char*)"winner: ", playerName);
		return "winner: " + playerName;
	}
	else {
		return "";
	}
	
}

// counts instances of a character in a string
// Ingame: Used to find the number of answers required for statement
int countChars(char* l, char s) {
	int elements = 0;
	for (int i = 0; i < sizeof(l)/sizeof(char); i++) {
		if (l[i] == s) {
			elements++;
		}
	}
	return elements;
}

// splits the string "target" into the "parts" array by the "delim" characters
// note: must know the size of "parts" you will need before calculation
//		 otherwise provide a bigger than necessary array for parts
void splitString(char* parts[], char* target, const char* delim) {
	char* tok;
	tok = std::strtok(target, delim);
	int i = 0;
	while(tok != NULL) {
		parts[i] = tok;
		tok = strtok(NULL, delim);
		i++;
	}
}