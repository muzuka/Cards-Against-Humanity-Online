/*
 *  The client acts as the player in the game.
 *  
 *  Written by: Sean Brown
 *	UCID: 10062604
 *  
 *  Features:
 *		+ Send messages to server
 *		+ Can get an appropriate name from server
 *		+ Receives messages from server
 *		+ Basic player and client communication possible
 *		+ Step 1-3 of the game
 *	To be implemented:
 *		- Steps 4-6 of the game from the client's perspective
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
int parseMessage(string);
string composeSENDMessage(char, Card);
string composeREQUESTMessage();
string composeNOTIFYMessage(char, string);
string substitute(string, string, char);
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
	bytesSent = send(self.getSocket(), self.getName().c_str(), self.getName().length(), 0);
	if (bytesSent <= 0) {
		printf("Couldn't send name.\n");
		exit(1);
	}
	
	// while sent name isn't available
	while(!isReady) {
		//memset(&inBuffer, 0, sizeof(inBuffer));
		bytesRecv = 0;
		while(bytesRecv <= 0) {
			// receive confirmation
			bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 1, 0);
		}
		printf("Received answer from Server: %s\n", (char*)inBuffer);
		if (inBuffer[0] == 'y') {
			break;
		}
		else if(inBuffer[0] == 'n'){
			printf("The name was already taken, please enter another:\n");
			cin >> name;
			self.setName(name);
		}
		
	}
	printf("The name was accepted!\n");
	// Receive cards
	for(int i = 0; i < 10; i++) {
		memset(&inBuffer, 0, sizeof(inBuffer));
		bytesRecv = 0;
		while(bytesRecv <= 0) {
			bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
		}
		parseMessage(string(inBuffer));
	}
	
	if(self.getHand().empty()) {
		 printf("Hand is empty.\n");
	}
	else {
		printf("This is your current hand.\nPlease enter the number to choose one when prompted.\n");
		printHand(self.getHand());
	}
	// Start main loop
	while(!isDone) {
		
		memset(&inBuffer, 0, sizeof(inBuffer));
		// receive NOTIFY for new judge
		bytesRecv = 0;
		while(bytesRecv <= 0) {
			bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
		}
		parseMessage(string(inBuffer));
		
		memset(&inBuffer, 0, sizeof(inBuffer));
		// receive POST for new black card
		bytesRecv = 0;
		while(bytesRecv <= 0) {
			bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
		}
		parseMessage(string(inBuffer));
		
		if(isJudge) {
			printf("Please wait for the others' answers.\n");
			
			memset(&inBuffer, 0, sizeof(inBuffer));
			bytesRecv = 0;
			while (bytesRecv <= 0) {
				bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
			}
			//printf("About to parse notify.\n");
			int playerNum = parseMessage(string(inBuffer));
			
			printf("Got all of the answers");
			for (int i = 0; i < playerNum; i++) {
				memset(&inBuffer, 0, sizeof(inBuffer));
				bytesRecv = 0;
				while (bytesRecv <= 0) {
					bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
				}
				parseMessage(string(inBuffer));
			}
			
			printf("All of the answers are ready.\n");
			printHand(answers);
			printf("Please select the best answer:\nCommands:\n");
			printf("a # to send answer\nt # to test answer");
		}
		else {
			bool good = false;
			int num;
			string answer, testAnswer;
			printf("Please enter your answer in this form:\na # to send answer.\nt # to test the answer.\n");
			printf("q to quit\nh to see hand.\n");
			cin >> answer;
			cin >> num;
			
			// Make a choice
			while (!good) {
				if (answer[0] == 't') {
					testAnswer = substitute(blackCard.content, self.getHand()[num-1].content, '_');
					printf("%s\n", testAnswer.c_str());
				}
				else if(answer[0] == 'a') {
					bytesSent = send(self.getSocket(), (char*)composeSENDMessage('n', self.takeCard(num-1)).c_str(), 100, 1);
					if (bytesSent <= 0) {
						printf("Couldn't send answer.\n");
						exit(1);
					}
					
					break;
				}
				else if (answer[0] == 'q') {
					exit(1);
				}
				else if(answer[0] == 'h') {
					printHand(self.getHand());
				}
				else {
					printf("Input not understood.\n");
				}
				
				printf("Please enter your answer in this form:\na # to send answer.\nt # to test the answer.\n");
				printf("q to quit\nh to see hand.\n");
				cin >> answer;
				cin >> num;
			}
			
			bytesSent = send(self.getSocket(), (char*)composeREQUESTMessage().c_str(), 100, 0);
			if (bytesSent <= 0) {
				printf("Couldn't send request.\n");
				exit(1);
			}
			
			bytesRecv = 0;
			memset(&inBuffer, 0, sizeof(inBuffer));
			while (bytesRecv <= 0) {
				bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
			}
			parseMessage(string(inBuffer));
			
			
		}
	}
}

// prints vector of cards
void printHand(vector<Card> hand) {
	for (int i = 0; i < hand.size(); i++) {
		printf("%d %s\n", i+1, hand[i].content.c_str());
	}
}

// Given a string, parseMessage parses it and does the action required by the message.
/*
 * ADD: adds to hand
 * POST: replaces black card
 * ANSWER: adds card to answers
 * NOTIFY: updates newest judge or winner
 */
int parseMessage(string message) {
	char* notifyMessage[2]; // [0] has option, [1] has name
	char* messageDiv[2]; // [0] has first half, [1] has content
	char* getCommand[2]; // [0] has command, [1] has source
	splitString(messageDiv, (char*)message.c_str(), "\n");
	splitString(getCommand, messageDiv[0], " ");
	
	if(strcmp((const char*)getCommand[0], "ADD") == 0) {
		//printf("Got an ADD message.\n");
		self.addCard(Card(string(messageDiv[1]), self));
		printf("Got a new card.\n");
	}
	else if(strcmp((const char*)getCommand[0], "POST") == 0) {
		blackCard = Card(string(messageDiv[1]), 'b', countChars(messageDiv[1], '_'));
		printf("Received a new black card:\n\"%s\"\n", messageDiv[1]);
	}
	else if(strcmp((const char*)getCommand[0], "ANSWER") == 0) {
		answers.push_back(Card(string(messageDiv[1]), Player(string(getCommand[1]), 0)));
	}
	else if(strcmp((const char*)getCommand[0], "NOTIFY") == 0) {
		splitString(notifyMessage, messageDiv[1], " ");
		if(strcmp((const char*)notifyMessage[0], "CP:") == 0) {
			if (strcmp(self.getName().c_str(), (const char*)notifyMessage[1]) == 0) {
				isJudge = !isJudge;
				judge = self;
				printf("You are the new judge!\n");
			}
			else {
				judge.setName(string(notifyMessage[1]));
				printf("The new judge is %s.\n", notifyMessage[1]);
			}
		}
		else if(strcmp((const char*)notifyMessage[0], "winner:") == 0) {
			if(strcmp(self.getName().c_str(), (const char*)notifyMessage[1]) == 0) {
				printf("You have won this round!\n");
				self.addPoint();
			}
		}
		else if(strcmp((const char*)notifyMessage[0], "player:") == 0) {
			printf("Receiving %d answers.\n", atoi(notifyMessage[1]));
			return atoi(notifyMessage[1]);
		}
	}
}

// composes a message to send, made of the card and source
/*
 * messag types: 'p' for post, 'n' for answer, 'd' for add
 */
string composeSENDMessage(char type, Card cardToSend) {
	char* t;
	if(type == 'n') {
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

string composeREQUESTMessage() {
	return "REQUEST " + self.getName() + "\n";
}

// Composes a message to notify, made of the info to send
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

// substitutes a character instances 
string substitute(string target, string replacement, char subChar) {
	string newString = target;
	for (int i = 0; i < newString.length(); i++) {
		if (newString[i] == subChar) {
			newString = newString.substr(0, i) + replacement + newString.substr(i+1);
		}
	}
	return newString;
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