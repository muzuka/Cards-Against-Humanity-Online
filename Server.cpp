/*
 *	The server implementation of Cards Against Humanity
 *	This handles communication between clients and runs the game.
 * 
 *	Written by: Sean Brown
 *	UCID: 10062604
 *
 *	Features:
 *		+ Accepts any client connection
 *		+ Sends messages to clients
 *		+ Can receive messages from client
 *		+ Implements all steps of the game
 *		+ New players are added once current turn is finished.
 *		
 *
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include "Card.h"
#include "Player.h"

using namespace std;

const int MAXPLAYERS = 10;

// for select Server
int maxDesc = 0;
fd_set recvSockSet;
bool terminated = false;


Player judge, winner;
Card blackCard;
int bytesRecv;
int bytesSent;
char inBuffer[200];
char outBuffer[200];
vector<Card> answers;
vector<Card> discard;
vector<Card> blackDeck;
vector<Card> whiteDeck;
vector<Player> players;


void initServer(int&, int);
int countChars(string, char);
int parseAnswer(string, vector<Card>&);
int parseMessage(string);
vector<Card> shuffle(vector<Card>, int);
string composeSENDMessage(char, Card);
vector<Player> shuffle(vector<Player>, int);
string composeNOTIFYMessage(char, string);
void shutdown();
void refill();
Player getPlayer(string);
int getPlayerIndex(string);
void deletePlayer(string);
void splitString(char**, char*, const char*);


int main(int argc, char *argv[]) {
	
	int serverSock;
	int clientSock;
	struct sockaddr_in clientAddr;
	
	struct timeval timeout = {0, 10};
	struct timeval selectTime;
	fd_set tempRecvSockSet;
	
	blackDeck.clear();
	whiteDeck.clear();
	discard.clear();
	players.clear();
	
	if (argc != 4) {
		printf("Usage: %s <Listening Port> <Black Deck File> <White Deck File>\n", argv[0]);
		exit(1);
	}
	
	initServer(serverSock, atoi(argv[1]));
	
	string line;
	ifstream blackCardReader, whiteCardReader;
	blackCardReader.open(argv[2]);
	whiteCardReader.open(argv[3]);
	
	
	if (!blackCardReader.is_open() || !whiteCardReader.is_open()) {
		printf("File open failed\n");
		blackCardReader.close();
		whiteCardReader.close();
		exit(1);
	}
	
	while(getline(blackCardReader, line)) {
		char* lineDiv[2];
		if (line[0] == 'q') {
			splitString(lineDiv, (char*)line.c_str(), "\"");
			string temp(lineDiv[1]);
			Card nBlack(temp, 'b', 1);
			blackDeck.push_back(nBlack);
		}
		else if (line[0] == 's') {
			splitString(lineDiv, (char*)line.c_str(), "\"");
			string temp(lineDiv[1]);
			int answers = countChars(lineDiv[1], '_');
			Card nBlack(temp, 'b', answers);
			blackDeck.push_back(nBlack);
		}
	}
	
	while (getline(whiteCardReader, line)) {
		Card nWhite(line, 'w', 0);
		whiteDeck.push_back(nWhite);
	}
	
	blackDeck = shuffle(blackDeck, atoi(argv[1]));
	whiteDeck = shuffle(whiteDeck, atoi(argv[1]));
	FD_ZERO(&recvSockSet);
	
	FD_SET(serverSock, &recvSockSet);
	maxDesc = max(maxDesc, serverSock);
	
	while (!terminated) {
		
		memcpy(&tempRecvSockSet, &recvSockSet, sizeof(recvSockSet));
		
		selectTime = timeout;
		int ready = select(maxDesc + 1, &tempRecvSockSet, NULL, NULL, &selectTime);
		if (ready < 0) {
			printf("select() failed\n");
			break;
		}
		
		if (FD_ISSET(serverSock, &tempRecvSockSet)) {
			unsigned int size = sizeof(clientAddr);
			
			if ((clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, &size)) < 0) {
				break;
			}
			printf("A new player has connected.\n");
			
			bool nameOK = false;
			bool foundName = false;
			
			while (!nameOK) {
				
				memset(&inBuffer, 0, sizeof(inBuffer));
				bytesRecv = 0;
				while (bytesRecv <= 0) {
					bytesRecv = recv(clientSock, (char*)&inBuffer, 20, 0);
				}
				printf("%s\n", inBuffer);
				
				if (players.size() < 1) {
					printf("Players is Empty.\n");
					outBuffer[0] = 'y';
					printf("Name hasn't been taken.\n");
					bytesSent = send(clientSock, (char*)&outBuffer, 1, 0);
					if (bytesSent <= 0) {
						printf("Couldn't send confirmation.\n");
						exit(1);
					}
					nameOK = !nameOK;
				}
				else {
					printf("Players isn't empty.\n");
					foundName = false;
					for (int i = 0; i < players.size(); i++) {
						if (players[i].getName().compare(inBuffer) == 0) {
							outBuffer[0] = 'n';
							bytesSent = 0;
							printf("Name is already taken.\n");
							bytesSent = send(clientSock, (char*)&outBuffer, 1, 0);
							if (bytesSent <= 0) {
								printf("Couldn't send denial.\n");
								exit(1);
							}
							foundName = true;
							break;
						}
					}
					if (!foundName) {
						outBuffer[0] = 'y';
						printf("Name hasn't been taken.\n");
						bytesSent = send(clientSock, (char*)&outBuffer, 1, 0);
						if (bytesSent <= 0) {
							printf("Couldn't send confirmation.\n");
							exit(1);
						}
						nameOK = !nameOK;
					}
				}
			}
			printf("Name checks out.\n");
			string n(inBuffer);
			Player newPlayer(n, clientSock);
			players.push_back(newPlayer);
			
			printf("About to send Cards.\n");
			for (int i = 0; i < 10; i++) {
				
				bytesSent = 0;
				if (whiteDeck.size() == 0) {
					refill();
				}
				string out = composeSENDMessage('d', whiteDeck[whiteDeck.size()-1]);
				players[players.size()-1].addCard(whiteDeck[whiteDeck.size()-1]);
				whiteDeck.pop_back();
				bytesSent = send(clientSock, (char*)out.c_str(), 100, 0);
				if (bytesSent <= 0) {
					printf("No Card was sent.\n");
					exit(1);
				}
			}
			
			FD_SET(clientSock, &recvSockSet);
			maxDesc = max(maxDesc, clientSock);
		}
		else {
			if(players.size() > 3) {
				string message;
				switch (step % 3) {
						// Choose judge and NOTIFY the others
					case 0:
						printf("Choosing a judge.\n");
						players = shuffle(players, atoi(argv[1])-step);
						judge = players[0];
						
						message = composeNOTIFYMessage('j', judge.getName());
						for (int i = 0; i < players.size(); i++) {
							bytesSent = send(players[i].getSocket(), (char*)message.c_str(), 100, 0);
							if (bytesSent <= 0) {
								printf("Judge message couldn't be sent.\n");
								exit(1);
							}
						}
						
						step++;
						break;
						// choose black card and POST it
					case 1:
						if (blackDeck.size() == 0) {
							shutdown();
						}
						printf("Choosing a black card.\n");
						blackCard = blackDeck[blackDeck.size() - 1];
						blackDeck.pop_back();
						
						message = composeSENDMessage('p', blackCard);
						for (int i = 0; i < players.size(); i++) {
							bytesSent = send(players[i].getSocket(), (char*)message.c_str(), 300, 0);
							if(bytesSent <= 0) {
								printf("Couldn't post black card.\n");
								exit(1);
							}
						}
						step++;
						break;
						// Handles answers and requests
					case 2:
						answers.clear();
						// Receive answers from others
						for (int i = 1; i < players.size(); i++) {
							for (int j = 0; j < blackCard.numOfAnswers; j++) {
								memset(&inBuffer, 0, sizeof(inBuffer));
								bytesRecv = 0;
								while(bytesRecv <= 0) {
									bytesRecv = recv(players[i].getSocket(), (char*)&inBuffer, 100, 0);
								}
								printf("player %d sent:\n%s.\n", i, (char*)string(inBuffer).c_str());
								parseAnswer(string(inBuffer), answers);
							}
						}
						printf("Received all answers.\n");
						// send expected # of answers to judge
						message = composeNOTIFYMessage('n', string("Server"));
						printf("About to send:\n%s\n", message.c_str());
						bytesSent = send(judge.getSocket(), (char*)message.c_str(), 100, 0);
						if(bytesSent <= 0) {
							printf("Failed to send NOTIFY to judge.\n");
							exit(1);
						}
						printf("Sent answers size.\n");
						// Send answers to judge
						for (int i = 0; i < answers.size(); i++) {
							message = composeSENDMessage('n', answers[i]);
							printf("About to send ANSWER to judge:\n%s\n", message.c_str());
							bytesSent = send(judge.getSocket(), (char*)message.c_str(), 100, 0);
							if(bytesSent <= 0) {
								printf("Failed to send ANSWER to judge.\n");
								i--;
							}
						}
						printf("Sent Answers to judge.\n");
						// Receive requests
						for (int i = 1; i < players.size(); i++) {
							for (int j = 0; j < blackCard.numOfAnswers; j++) {
								memset(&inBuffer, 0, sizeof(inBuffer));
								bytesRecv = 0;
								while (bytesRecv <= 0) {
									bytesRecv = recv(players[i].getSocket(), (char*)&inBuffer, 100, 0);
								}
								parseMessage(string(inBuffer));
							}
						}
						printf("Received all of the requests from clients.\n");

						// receives winner
						bytesRecv = 0;
						memset(&inBuffer, 0, sizeof(inBuffer));
						while (bytesRecv <= 0) {
							bytesRecv = recv(judge.getSocket(), (char*)&inBuffer, 100, 0);
						}
						parseMessage(string(inBuffer));

						// Tell everybody else
						for (int i = 1; i < players.size(); i++) {
							string winningMesssage = composeNOTIFYMessage('w', winner.getName());
							bytesSent = send(players[i].getSocket(), (char*)winningMesssage.c_str(), 100, 0);
							if(bytesSent <= 0) {
								printf("Failed to winner message.\n");
								exit(1);
							}
						}
						step++;
						break;
					default:
						break;
				}
			}
		}
	}
}

// Initializes the server
// Needs the socket and port
void initServer(int& serverSock, int port) {
	struct sockaddr_in serverAddr;
	
	if((serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("socket() failed\n");
		exit(1);
	}
	
	int yes = 1;
	if(setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
		printf("setsockopt() failed\n");
		exit(1);
	}
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(serverSock, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		printf("bind() failed\n");
		exit(1);
	}
	
	if(listen(serverSock, MAXPLAYERS) < 0) {
		printf("listen() failed\n");
		exit(1);
	}
}

// counts instances of a character in a string
// Ingame: Used to find the number of answers required for statement
int countChars(string l, char s) {
	int elements = 0;
	for (int i = 0; i < l.length(); i++) {
		if(l[i] == s) {
			elements++;
		}
	}
	return elements;
}

// Parses ANSWER message
int parseAnswer(string message, vector<Card> &answers) {
	char* messageDiv[2]; // [0] has first half, [1] has content
	char* getCommand[2]; // [0] has command, [1] has source
	splitString(messageDiv, (char*)message.c_str(), "\n");
	splitString(getCommand, messageDiv[0], " ");
	
	if(strcmp((const char*)getCommand[0], "ANSWER") == 0 || strcmp((const char*)getCommand[0], "NSWER") == 0) {
		players[getPlayerIndex(string(getCommand[1]))].takeCard(string(messageDiv[1]));
		answers.push_back(Card(string(messageDiv[1]), string(getCommand[1])));
		return 1;
	}
	else {
		return -1;
	}
	
}

int parseMessage(string message) {
	char* notifyMessage[2]; // [0] has option, [1] has name
	char* messageDiv[2]; // [0] has type and source, [1] has content
	char* getCommand[2]; // [0] has command, [1] has source
	splitString(messageDiv, (char*)message.c_str(), "\n");
	splitString(getCommand, messageDiv[0], " ");
	
	if(strcmp((const char*)getCommand[0], "REQUEST") == 0) {
		Player newP = getPlayer(string(getCommand[1]));
		if (whiteDeck.size() == 0) {
			refill();
		}
		string add = composeSENDMessage('d', whiteDeck[whiteDeck.size() - 1]);
		whiteDeck.pop_back();
		bytesSent = send(newP.getSocket(), (char*)add.c_str(), 100, 0);
		return 1;
	}
	else if(strcmp((const char*)getCommand[0], "NOTIFY") == 0) {
		splitString(notifyMessage, messageDiv[1], " ");
		if (strcmp(notifyMessage[0], "winner:") == 0) {
			for (int i = 0; i < answers.size(); i++) {
				discard.push_back(answers[i]);
			}
			answers.clear();
			winner = getPlayer(string(notifyMessage[1]));
		}
		else if(strcmp(notifyMessage[0], "quit:") == 0) {
			Player temp = getPlayer(string(notifyMessage[1]));
			for(int i = 0; i < temp.getHand().size(); i++) {
				discard.push_back(temp.getHand()[i]);
			}
			close(temp.getSocket());
			deletePlayer(string(notifyMessage[1]));
			if (players.size() == 0)
				exit(1);
		}
		return 1;
	}
	else {
		return -1;
	}
	
}

// shuffles the elements in the deck
vector<Card> shuffle(vector<Card> deck, int seed) {
	vector<Card> newDeck;
	newDeck.clear();
	
	for (int i = 1; i < deck.size(); i+=2) {
		newDeck.push_back(deck[i]);
	}
	for (int i = 0; i < deck.size(); i+=2) {
		newDeck.push_back(deck[i]);
	}
	if(seed == 0) {
		return newDeck;
	}
	else {
		return shuffle(newDeck, seed - 1);
	}
}

// composes a message to send, made of the card and source
/*
 * messag types: 'p' for post, 'n' for answer, 'd' for add
 */
string composeSENDMessage(char type, Card cardToSend) {
	if(type == 'p') {
		//printf("About to send POST:\n%s\n", cardToSend.content.c_str());
		return "POST Server\n" + cardToSend.content;
	}
	else if(type == 'n') {
		//printf("About to send ANSWER:\n%s %s\n", cardToSend.owner.c_str(), cardToSend.content.c_str());
		return "ANSWER " + cardToSend.owner + "\n" + cardToSend.content;
	}
	else if (type == 'd') {
		//printf("About to send ADD:\n%s\n", cardToSend.content.c_str());
		return "ADD Server\n" + cardToSend.content;
	}
	else {
		return "";
	}
	
}

// shuffles the elements in the deck
vector<Player> shuffle(vector<Player> deck, int seed) {
	vector<Player> newDeck;
	newDeck.clear();
	
	for (int i = 1; i < deck.size(); i+=2) {
		newDeck.push_back(deck[i]);
	}
	for (int i = 0; i < deck.size(); i+=2) {
		newDeck.push_back(deck[i]);
	}
	if (seed == 0) {
		return newDeck;
	}
	else {
		return shuffle(newDeck, seed - 1);
	}
}

// Composes a message to notify, made of the info to send
/*
 *
 */
string composeNOTIFYMessage(char purpose, string playerName) {
	if (purpose == 'j') {
		return "NOTIFY Server\nCP: " + playerName;
	}
	else if (purpose == 'w') {
		return "NOTIFY Server\nwinner: " + playerName;
	}
	else if(purpose == 'n') {
		stringstream s;
		s << answers.size();
		return "NOTIFY Server\nplayers: " + s.str();
	}
	else if(purpose == 'q') {
		return "NOTIFY Server\nquit: Server";
	}
	else {
		return "";
	}
	
}

void shutdown() {
	for (int i = 0; i < players.size(); i++) {
		string quit = composeNOTIFYMessage('q', "");
		bytesSent = send(players[i].getSocket(), (char*)quit.c_str(), 100, 0);
		if(bytesSent <= 0) {
			printf("Couldn't send quit.\n");
			exit(1);
		}
	}	
	exit(1);
}

// finds player in a the list of Players
Player getPlayer(string name) {
	for (int i = 0; i < players.size(); i++) {
		if (strcmp(players[i].getName().c_str(), name.c_str()) == 0) {
			return players[i];
		}
	}
	return Player();
}

int getPlayerIndex(string name) {
	for (int i = 0; i < players.size(); i++) {
		if (strcmp(players[i].getName().c_str(), name.c_str()) == 0) {
			return i;
		}
	}
	return -1;
}

void deletePlayer(string playerName) {
	vector<Player> temp;
	for (int i = 0; i < players.size(); i++) {
		if (strcmp(players[i].getName().c_str(), playerName.c_str()) != 0) {
			temp.push_back(players[i]);
		}
		else {
			delete &players[i];
		}
		
	}
	players = temp;
}

// refills the white deck when it runs out
void refill() {
	whiteDeck = discard;
	whiteDeck = shuffle(whiteDeck, blackDeck.size());
	discard.clear();
}

// splits the string "target" into the "parts" array by the "delim" characters
// note: must know the size of "parts" you will need before calculation
//		 otherwise provide a bigger than necessary array for parts
void splitString(char* parts[], char* target, const char* delim) {
	char* tok;
	tok = std::strtok(target, delim);
	int i = 0;
	while (tok != NULL) {
		parts[i] = tok;
		tok = strtok(NULL, delim);
		i++;
	}
}
