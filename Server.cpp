
/*
 * The server implementation of Cards Against Humanity
 * This handles communication between clients and runs the game.
 * Usage: 
 *
 */

#include <sys/socket.h>
#include <arpa/inet.h>
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

int step = 1;
Player judge;
Card blackCard;
int bytesRecv = 0;
int bytesSent = 0;
char inBuffer[200];
char outBuffer[200];
vector<Card> discard;
vector<Card> blackDeck;
vector<Card> whiteDeck;
vector<Player> players;


void initServer(int&, int);
int countChars(char*, char);
vector<Card> shuffle(vector<Card>);
string composeSENDMessage(char, Card);
vector<Player> shuffle(vector<Player>);
string composeNOTIFYMessage(char, string);
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
	
	if(argc != 4) {
		printf("Usage: %s <Listening Port> <Black Deck File> <White Deck File>\n", argv[0]);
		exit(1);
	}
	
	initServer(serverSock, atoi(argv[1]));
	
	string line;
	ifstream blackCardReader, whiteCardReader;
	blackCardReader.open(argv[2]);
	whiteCardReader.open(argv[3]);

	
	if(!blackCardReader.is_open() || !whiteCardReader.is_open()) {
		printf("File open failed\n");
		blackCardReader.close();
		whiteCardReader.close();
		exit(1);
	}
	
	while(getline(blackCardReader, line)) {
		char* lineDiv[2];
		if(line[0] == 'q') {
			splitString(lineDiv, (char*)line.c_str(), "\"");
			string temp(lineDiv[1]);
			Card nBlack(temp, 'b', 0);
			blackDeck.push_back(nBlack);
		}
		else if(line[0] == 's') {
			splitString(lineDiv, (char*)line.c_str(), "\n");
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
	
	blackDeck = shuffle(blackDeck);
	whiteDeck = shuffle(whiteDeck);
	FD_ZERO(&recvSockSet);
	
	FD_SET(serverSock, &recvSockSet);
	maxDesc = max(maxDesc, serverSock);
	
	while (!terminated) {
		
		memcpy(&tempRecvSockSet, &recvSockSet, sizeof(recvSockSet));
		
		selectTime = timeout;
		int ready = select(maxDesc + 1, &tempRecvSockSet, NULL, NULL, &selectTime);
		if(ready < 0) {
			printf("select() failed\n");
			break;
		}
		
		if(FD_ISSET(serverSock, &tempRecvSockSet)) {
			unsigned int size = sizeof(clientAddr);
			
			if((clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, &size)) < 0) {
				break;
			}
			printf("A new player has connected.\n");
			
			bool nameOK = false;
			bool foundName = false;
			
			while(!nameOK) {
			
				bytesRecv = 0;
				while(bytesRecv <= 0) {
					bytesRecv = recv(clientSock, (char*)&inBuffer, 20, 0);
				}
				printf("%s\n", inBuffer);
			
				if(players.size() <= 1) {
					outBuffer[0] = 'y';
					bytesSent = send(clientSock, (char*)&outBuffer, 1, 0);
					if(bytesSent <= 0) {
						printf("Couldn't send confirmation.\n");
						exit(1);
					}
					nameOK = !nameOK;
				}
				else {
					for(int i = 0; i < players.size(); i++) {
						if(players[i].getName().compare(inBuffer) == 0) {
							outBuffer[0] = 'n';
							bytesSent = 0;
							bytesSent = send(clientSock, (char*)&outBuffer, 1, 0);
							if(bytesSent <= 0) {
								printf("Couldn't send denial.\n");
								exit(1);
							}
							foundName = true;
							break;
						}
					}
					if(foundName) {
						nameOK = !nameOK;
					}
				}
				printf("Name checks out.\n");
				string n(inBuffer);
				Player newPlayer(n, clientSock);
				players.push_back(newPlayer);
				
				printf("About to send Cards.\n");
				for(int i = 0; i < 10; i++) {
					
					bytesSent = 0;
					string out = composeSENDMessage('d', whiteDeck[whiteDeck.size()-1]);
					whiteDeck.pop_back();
					printf("About to send card.\n");
					bytesSent = send(clientSock, (char*)out.c_str(), 100, 0);
					if(bytesSent <= 0) {
						printf("No Card was sent.\n");
						exit(1);
					}
					printf("Sent Card.\n");
				}
				
			}
			FD_SET(clientSock, &recvSockSet);
			maxDesc = max(maxDesc, clientSock);
		}
		else {
			if(players.size() > 2) {
				string message;
				switch (step % 6) {
					case 1:
						players = shuffle(players);
						judge = players[0];
						
						message = composeNOTIFYMessage('j', judge.getName());
						for(int i = 0; i < players.size(); i++) {
							bytesSent = send(players[i].getSocket(), (char*)message.c_str(), 100, 0);
							if(bytesSent <= 0) {
								printf("Judge message couldn't be sent.\n");
								exit(1);
							}
						}
						blackCard = blackDeck[blackDeck.size() - 1];
						blackDeck.pop_back();
						
						message = composeSENDMessage('p', blackCard);
						for (int i = 0; i < players.size(); i++) {
							bytesSent = send(players[i].getSocket(), (char*)message.c_str(), 100, 0);
							if(bytesSent <= 0) {
								printf("Couldn't post black card.\n");
								exit(1);
							}
						}
						step++;
						break;
					case 2:
						
						break;
					case 3:
						
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
int countChars(char* l, char s) {
	int elements = 0;
	for (int i = 0; i < sizeof(l)/sizeof(char); i++) {
		if (l[i] == s) {
			elements++;
		}
	}
	return elements;
}

// shuffles the elements in the deck
vector<Card> shuffle(vector<Card> deck) {
	vector<Card> newDeck;
	newDeck.clear();
	
	for(int i = 0; i < deck.size(); i+=2) {
		newDeck.push_back(deck[i]);
	}
	for (int i = 1; i < deck.size(); i+=2) {
		newDeck.push_back(deck[i]);
	}
	return newDeck;
}

// composes a message to send, made of the card and source
/*
 * messag types: 'p' for post, 'n' for answer, 'd' for add
 */
string composeSENDMessage(char type, Card cardToSend) {
	if(type == 'p') {
		//return strcat((char*)"POST Server\n", cardToSend.content.c_str());
		return "POST Server\n" + cardToSend.content;
	}
	else if(type == 'n') {
		//return strcat((char*)"ANSWER Server\n", cardToSend.content.c_str());
		return "ANSWER Server\n" + cardToSend.content;
	}
	else if(type == 'd') {
		//return strcat((char*)"ADD Server\n", cardToSend.content.c_str());
		return "ADD Server\n" + cardToSend.content;
	}
	else {
		return "";
	}

}

// shuffles the elements in the deck
vector<Player> shuffle(vector<Player> deck) {
	vector<Player> newDeck;
	newDeck.clear();
	
	for(int i = 0; i < deck.size(); i+=2) {
		newDeck.push_back(deck[i]);
	}
	for (int i = 1; i < deck.size(); i+=2) {
		newDeck.push_back(deck[i]);
	}
	return newDeck;
}

// Composes a message to notify, made of the info to send
/*
 *
 */
string composeNOTIFYMessage(char purpose, string playerName) {
	if(purpose == 'j') {
		//return strcat((char*)"CP: ", playerName);
		return "NOTIFY Server\nCP: " + playerName;
	}
	else if(purpose == 'w') {
		//return strcat((char*)"winner: ", playerName);
		return "NOTIFY Server\nwinner: " + playerName;
	}
	else {
		return "";
	}

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
