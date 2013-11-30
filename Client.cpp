/*
 *  The client acts as the player in the game.
 *  
 *  Written by: Sean Brown
 *        UCID: 10062604
 *  
 *  Features:
 *                + Send messages to server
 *                + Can get an appropriate name from server
 *                + Receives messages from server
 *                + Basic player and client communication possible
 *                + All steps of the game finished
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
void printAnswers(vector<Card>, int);
int parseMessage(string);
string composeSENDMessage(char, Card);
string composeREQUESTMessage();
string composeNOTIFYMessage(char, string);
string substitute(string, string[], char);
int countChars(string, char);
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
	printf("What do you want as a username(Must be unique and have no spaces):\n");
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
		memset(&inBuffer, 0, sizeof(inBuffer));
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
			bytesSent = send(self.getSocket(), self.getName().c_str(), self.getName().length(), 0);
			if (bytesSent <= 0) {
				printf("Couldn't send name.\n");
				exit(1);
			}
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
		
		printf("A new round has begun!\n");
		
		// receive NOTIFY for new judge
		while (true) {
			memset(&inBuffer, 0, sizeof(inBuffer));
			bytesRecv = 0;
			while(bytesRecv <= 0) {
				bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
			}
			if (string(inBuffer).length() != 0) {
				printf("The new judge is %s.\n", string(inBuffer).c_str());
				parseMessage(string(inBuffer));
				break;
			}
		}
		
		// receive POST for new black card
		while (true) {
			memset(&inBuffer, 0, sizeof(inBuffer));
			bytesRecv = 0;
			while(bytesRecv <= 0) {
				bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 300, 0);
			}
			if (string(inBuffer).length() != 0) {
				printf("The black card is %s.\n", string(inBuffer).c_str());
				parseMessage(string(inBuffer));
				break;
			}
		}
		
		string ans[3];
		int num[3];
		char answer;
		string testAnswer;
		
		// Judge Code --------------------------------------
		if(isJudge) {
			printf("Please wait for the others' answers.\n");
			int playerNum = 0;
			// Get number of answers to expect.
			while (playerNum <= 0) {
				memset(&inBuffer, 0, sizeof(inBuffer));
				bytesRecv = 0;
				while (bytesRecv <= 0) {
					bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
				}
				if (string(inBuffer).length() != 0) {
					printf("About to parse notify.\n%s\n", string(inBuffer).c_str());
					playerNum = parseMessage(string(inBuffer));
				}
				else {
					printf("Nothing received yet.\n");
				}
				
				
			}
			printf("PlayerNum is equal to %d.\n", playerNum);
			// receive answers
			for (int i = 0; i < playerNum; i++) {
				memset(&inBuffer, 0, sizeof(inBuffer));
				bytesRecv = 0;
				while (bytesRecv <= 0) {
					bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
				}
				printf("new answer(before parsing):\n%s\n", (char*)inBuffer);
				parseMessage(string(inBuffer));
			}
			
			printf("Got all %d of the answers.\n", playerNum);
			
			printf("All of the answers are ready.\n");
			printAnswers(answers, blackCard.numOfAnswers);
			printf("Please select the best answer:\nCommands:\n");
			printf("a # to send answer.\nt # to test answer.\nh to see choices.\ns to see score.\nq to quit.\n");
			
			string chosen, note;
			cin >> answer;
			if (answer == 'q') {
				note = composeNOTIFYMessage('q', self.getName());
				bytesSent = send(self.getSocket(), (char*)note.c_str(), 100, 0);
				if (bytesSent <= 0) {
					printf("Couldn't send quit.\n");
					exit(1);
				}
				exit(1);
			}
			else if (answer == 'h') {
				printAnswers(answers, blackCard.numOfAnswers);
			}
			else if (answer == 's') {
				printf("You have won %d rounds.\n", self.getScore());
			}
			else {
				cin >> num[0];
			}
			bool ready = false;
			while (!ready) {
				switch (answer) {
					case 'a':
						if(num[0] > blackCard.numOfAnswers || num[0] < 1) {
							printf("Choice is out of range. Try again.");
							break;
						}
						chosen = composeNOTIFYMessage('w', answers[blackCard.numOfAnswers*(num[0] - 1)].owner);
						bytesSent = send(self.getSocket(), (char*)chosen.c_str(), 100, 0);
						if (bytesSent <= 0) {
							printf("Couldn't send NOTIFY.\n");
							exit(1);
						}
						printf("Sent winner to server.\n");
						ready = !ready;
						break;
					case 't':
						int place;
						switch (blackCard.numOfAnswers) {
							case 1:
								ans[0] = answers[num[0]-1].content;
								testAnswer = substitute(blackCard.content, ans, '_');
								printf("%s\n", testAnswer.c_str());
								break;
							case 2:
								place = blackCard.numOfAnswers*(num[0]-1);
								ans[0] = answers[place].content;
								ans[1] = answers[place + 1].content;
								testAnswer = substitute(blackCard.content, ans, '_');
								printf("%s\n", testAnswer.c_str());
								break;
							case 3:
								place = blackCard.numOfAnswers*(num[0]-1);
								ans[0] = answers[place].content;
								ans[1] = answers[place + 1].content;
								ans[2] = answers[place + 2].content;
								testAnswer = substitute(blackCard.content, ans, '_');
								printf("%s\n", testAnswer.c_str());
								break;
							default:
								break;
						}
						break;
					default:
						break;
				}
				if (!ready) {
					printf("Please select the best answer:\nCommands:\n");
					printf("a # to send answer.\nt # to test answer.\nh to see hand\ns to see score\nq to quit.\n");
					
					cin >> answer;
					if (answer == 'q') {
						note = composeNOTIFYMessage('q', self.getName());
						bytesSent = send(self.getSocket(), (char*)note.c_str(), 100, 0);
						if (bytesSent <= 0) {
							printf("Couldn't send quit.\n");
							exit(1);
						}
						exit(1);
					}
					else if (answer == 'h') {
						printAnswers(answers, blackCard.numOfAnswers);
					}
					else if (answer == 's') {
						printf("You have won %d rounds.\n", self.getScore());
					}
					else {
						cin >> num[0];
					}
				}
			}
		}
		// Non-Judge Code---------------------------------------------------
		else {
			bool good = false;
			switch (blackCard.numOfAnswers) {
				case 1:
					printf("This black card requires one white card.\n");
					printf("Please enter your answer in this form:\na # to send answer.\nt # to test the answer.\n");
					printf("s to see score.\nb to see black card\nh to see hand.\nq to quit.\n");
					cin >> answer;
					if (answer == 'q') {
						string note = composeNOTIFYMessage('q', self.getName());
						bytesSent = send(self.getSocket(), (char*)note.c_str(), 100, 0);
						if (bytesSent <= 0) {
							printf("Couldn't send quit.\n");
							exit(1);
						}
						exit(1);
					}
					else if(answer == 'h') {
						printHand(self.getHand());
					}
					else if(answer == 's') {
						printf("You have won %d rounds.\n", self.getScore());
					}
					else if(answer == 'b') {
						printf("%s\n", blackCard.content.c_str());
					}
					else {
						cin >> num[0];
					}
					break;
				case 2:
					printf("This black card requires two white cards.\n");
					printf("Please enter your answer in this form:\na # # to send answer.\nt # # to test the answer");
					printf("s to see score.\nb to see black card\nh to see hand.\nq to quit.\n");
					cin >> answer;
					if (answer == 'q') {
						string note = composeNOTIFYMessage('q', self.getName());
						bytesSent = send(self.getSocket(), (char*)note.c_str(), 100, 0);
						if (bytesSent <= 0) {
							printf("Couldn't send quit.\n");
							exit(1);
						}
						exit(1);
					}
					else if(answer == 'h') {
						printHand(self.getHand());
					}
					else if(answer == 's') {
						printf("You have won %d rounds.\n", self.getScore());
					}
					else if(answer == 'b') {
						printf("%s\n", blackCard.content.c_str());
					}
					else {
						cin >> num[0];
						cin >> num[1];
					}
					break;
				case 3:
					printf("This black card requires three white cards.\n");
					printf("Please enter your answer in this form:\na # # # to send answer.\nt # # # to test the answer");
					printf("s to see score.\nb to see black card\nh to see hand.\nq to quit.\n");
					cin >> answer;
					if (answer == 'q') {
						string note = composeNOTIFYMessage('q', self.getName());
						bytesSent = send(self.getSocket(), (char*)note.c_str(), 100, 0);
						if (bytesSent <= 0) {
							printf("Couldn't send quit.\n");
							exit(1);
						}
						exit(1);
					}
					else if(answer == 'h') {
						printHand(self.getHand());
					}
					else if(answer == 's') {
						printf("You have won %d rounds.\n", self.getScore());
					}
					else if(answer == 'b') {
						printf("%s\n", blackCard.content.c_str());
					}
					else {
						cin >> num[0];
						cin >> num[1];
						cin >> num[2];
					}
					break;
				default:
					printf("%d\n", blackCard.numOfAnswers);
					break;
			}
			
			// Make a choice
			while (!good) {
				if (answer == 't') {
					switch (blackCard.numOfAnswers) {
						case 1:
							ans[0] = self.getHand()[num[0] - 1].content;
							testAnswer = substitute(blackCard.content, ans, '_');
							printf("%s\n", testAnswer.c_str());
							break;
						case 2:
							ans[0] = self.getHand()[num[0] - 1].content;
							ans[1] = self.getHand()[num[1] - 1].content;
							testAnswer = substitute(blackCard.content, ans, '_');
							printf("%s\n", testAnswer.c_str());
							break;
						case 3:
							ans[0] = self.getHand()[num[0] - 1].content;
							ans[1] = self.getHand()[num[1] - 1].content;
							ans[2] = self.getHand()[num[2] - 1].content;
							testAnswer = substitute(blackCard.content, ans, '_');
							printf("%s\n", testAnswer.c_str());
							break;
						default:
							break;
					}
				}
				else if(answer == 'a') {
					for (int i = 0; i < blackCard.numOfAnswers; i++) {
						string messageAnswer = composeSENDMessage('n', self.takeCard(num[i]-1));
						bytesSent = send(self.getSocket(), (char*)messageAnswer.c_str(), 100, 1);
						if (bytesSent <= 0) {
							printf("Couldn't send answer.\n");
							exit(1);
						}
					}
					break;
				}
				else if (answer == 'q' || answer == 'h' || answer == 's') {
					
				}
				
				switch (blackCard.numOfAnswers) {
					case 1:
						printf("This black card requires one  white card.\n");
						printf("Please enter your answer in this form:\na # to send answer.\nt # to test the answer.\n");
						printf("s to see score.\nb to see black card\nh to see hand.\nq to quit.\n");
						cin >> answer;
						if (answer == 'q') {
							string note = composeNOTIFYMessage('q', self.getName());
							bytesSent = send(self.getSocket(), (char*)note.c_str(), 100, 0);
							if (bytesSent <= 0) {
								printf("Couldn't send quit.\n");
								exit(1);
							}
							exit(1);
						}
						else if(answer == 'h') {
							printHand(self.getHand());
						}
						else if(answer == 's') {
							printf("You have won %d rounds.\n", self.getScore());
						}
						else if(answer == 'b') {
							printf("%s\n", blackCard.content.c_str());
						}
						else {
							cin >> num[0];
						}
						break;
					case 2:
						printf("This black card requires two white cards.\n");
						printf("Please enter your answer in this form:\na # # to send answer.\nt # # to test the answer");
						printf("s to see score.\nb to see black card\nh to see hand.\nq to quit.\n");
						cin >> answer;
						if (answer == 'q') {
							string note = composeNOTIFYMessage('q', self.getName());
							bytesSent = send(self.getSocket(), (char*)note.c_str(), 100, 0);
							if (bytesSent <= 0) {
								printf("Couldn't send quit.\n");
								exit(1);
							}
							exit(1);
						}
						else if(answer == 'h') {
							printHand(self.getHand());
						}
						else if(answer == 's') {
							printf("You have won %d rounds.\n", self.getScore());
						}
						else if(answer == 'b') {
							printf("%s\n", blackCard.content.c_str());
						}
						else {
							cin >> num[0];
							cin >> num[1];
						}
						break;
					case 3:
						printf("This black card requires three white cards.\n");
						printf("Please enter your answer in this form:\na # # # to send answer.\nt # # # to test the answer");
						printf("s to see score.\nb to see black card\nh to see hand.\nq to quit.\n");
						cin >> answer;
						if (answer == 'q') {
							string note = composeNOTIFYMessage('q', self.getName());
							bytesSent = send(self.getSocket(), (char*)note.c_str(), 100, 0);
							if (bytesSent <= 0) {
								printf("Couldn't send quit.\n");
								exit(1);
							}
							exit(1);
						}
						else if(answer == 'h') {
							printHand(self.getHand());
						}
						else if(answer == 's') {
							printf("You have won %d rounds.\n", self.getScore());
						}
						else if(answer == 'b') {
							printf("%s\n", blackCard.content.c_str());
						}
						else {
							cin >> num[0];
							cin >> num[1];
							cin >> num[2];
						}
						break;
					default:
						break;
				}
			}
			
			for (int i = 0; i < blackCard.numOfAnswers; i++) {
				bytesSent = send(self.getSocket(), (char*)composeREQUESTMessage().c_str(), 100, 0);
				if (bytesSent <= 0) {
					printf("Couldn't send request.\n");
					exit(1);
				}
			}
			
			for (int i = 0; i < blackCard.numOfAnswers; i++) {
				bytesRecv = 0;
				memset(&inBuffer, 0, sizeof(inBuffer));
				while (bytesRecv <= 0) {
					bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
				}
				if (string(inBuffer).length() != 0) {
					printf("Received new white card on request.\n");
					parseMessage(string(inBuffer));
				}
				else {
					i--;
				}
				
			}
			printf("Received all new cards as needed.\n");
			
			while (true) {
				memset(&inBuffer, 0, sizeof(inBuffer));
				bytesRecv = 0;
				while (bytesRecv <= 0) {
					bytesRecv = recv(self.getSocket(), (char*)&inBuffer, 100, 0);
				}
				if (string(inBuffer).length() != 0) {
					parseMessage(string(inBuffer));
					break;
				}
			}
		}
	}
}

// prints vector of cards
void printHand(vector<Card> hand) {
	for (int i = 0; i < hand.size(); i++) {
		printf("%d %s\n", i+1, hand[i].content.c_str());
	}
}

// Prints the Cards in the answers vector.
// It also keeps the answers together with the source
void printAnswers(vector<Card> answers, int numOfAnswers) {
	int count = 1;
	for (int i = 0; i < answers.size()/numOfAnswers; i++) {
		for (int j = 0; j < numOfAnswers; j++) {
			printf("%d %s\n", count, answers[i].content.c_str());
		}
		count++;
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
	char* messageDiv[2]; // [0] has type and source, [1] has content
	char* getCommand[2]; // [0] has command, [1] has source
	splitString(messageDiv, (char*)message.c_str(), "\n");
	splitString(getCommand, messageDiv[0], " ");
	
	if(strcmp((const char*)getCommand[0], "ADD") == 0) {
		self.addCard(Card(string(messageDiv[1]), self.getName()));
		//printf("Got a new card.\n");
		return 1;
	}
	else if(strcmp((const char*)getCommand[0], "POST") == 0) {
		blackCard = Card(string(messageDiv[1]), 'b', countChars(messageDiv[1], '_'));
		printf("Received a new black card:\n\"%s\"\n", messageDiv[1]);
		return 1;
	}
	else if(strcmp((const char*)getCommand[0], "ANSWER") == 0) {
		printf("New answer:\n");
		printf("%s\n", messageDiv[1]);
		answers.push_back(Card(string(messageDiv[1]), string(getCommand[1])));
		return 1;
	}
	else if(strcmp((const char*)getCommand[0], "NOTIFY") == 0) {
		splitString(notifyMessage, messageDiv[1], " ");
		if(strcmp((const char*)notifyMessage[0], "CP:") == 0) {
			if (strcmp(self.getName().c_str(), (const char*)notifyMessage[1]) == 0) {
				if (isJudge) {
					answers.clear();
					printf("You are the new judge!\n");
				}
				else {
					isJudge = !isJudge;
					judge = self;
					printf("You are the new judge!\n");
				}
			}
			else {
				if (isJudge) {
					isJudge = !isJudge;
					judge.setName(string(notifyMessage[1]));
					answers.clear();
					printf("The new judge is %s.\n", notifyMessage[1]);
				}
				else {
					judge.setName(string(notifyMessage[1]));
					printf("The new judge is %s.\n", notifyMessage[1]);
				}
			}
			
		}
		else if(strcmp((const char*)notifyMessage[0], "winner:") == 0) {
			if(strcmp(self.getName().c_str(), (const char*)notifyMessage[1]) == 0) {
				printf("You have won this round!\n");
				self.addPoint();
			}
		}
		else if(strcmp((const char*)notifyMessage[0], "players:") == 0) {
			printf("Receiving %d answers from %s.\n", atoi(notifyMessage[1]), notifyMessage[1]);
			if (atoi(notifyMessage[1]) == 0) {
				printf("No more players.\nQuitting now.\n");
				exit(1);
			}
			return atoi(notifyMessage[1]);
		}
		else if(strcmp((const char*)notifyMessage[0], "quit:") == 0) {
			exit(1);
		}
		else {
			return -1;
		}
		return 1;
	}
	else {
		return -1;
	}
	
}

// composes a message to send, made of the card and source
/*
 * messag types: 'p' for post, 'n' for answer, 'd' for add
 */
string composeSENDMessage(char type, Card cardToSend) {
	char* t;
	if(type == 'n') {
		return "ANSWER " + self.getName() + "\n" + cardToSend.content;
	}
	else if(type == 'd') {
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
		return "NOTIFY " + self.getName() + "\nCP: " + playerName;
	}
	else if(purpose == 'w') {
		return "NOTIFY " + self.getName() + "\nwinner: " + playerName;
	}
	else if(purpose == 'q') {
		return "NOTIFY " + self.getName() + "\nquit: " + playerName;
	}
	else {
		return "";
	}
	
}

// substitutes a character instances 
string substitute(string target, string replacement[], char subChar) {
	string newString = target;
	int s = 0;
	for (int i = 0; i < newString.length(); i++) {
		if (newString[i] == subChar) {
			newString = newString.substr(0, i) + replacement[s] + newString.substr(i+1);
			s++;
		}
	}
	return newString;
}

// counts instances of a character in a string
// Ingame: Used to find the number of answers required for statement
int countChars(string l, char s) {
	int elements = 0;
	for (int i = 0; i < l.length(); i++) {
		if (l[i] == s) {
			elements++;
		}
	}
	if (s == '_' && elements == 0) {
		return 1;
	}
	return elements;
}

// splits the string "target" into the "parts" array by the "delim" characters
// note: must know the size of "parts" you will need before calculation
//                 otherwise provide a bigger than necessary array for parts
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
