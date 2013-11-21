
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
fd_set recvSockSet;
int maxDesc = 0;
bool terminated = false;

vector<Card> blackDeck;
vector<Card> whiteDeck;
vector<Card> discard;
vector<Player> players;
Player* judge;
Card* blackCard;

void initServer(int&, int);
int countChars(char*, char);
void splitString(char**, char*, const char*);


int main(int argc, char *argv[]) {
  
	int serverSock;
	int clientSock;
	struct sockaddr_in clientAddr;
	
	struct timeval timeout = {0, 10};
	struct timeval selectTime;
	fd_set tempRecvSockSet;
	
	if(argc != 4) {
		printf("Usage: %s <Listening Port>\n", argv[0]);
		exit(1);
	}
	
	initServer(serverSock, atoi(argv[1]));
	
	string line;
	ifstream blackCardReader, whiteCardReader;
	blackCardReader.open(argv[3]);
	whiteCardReader.open(argv[4]);

	
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
			Card nBlack(lineDiv[1], 'b', 0);
			blackDeck.push_back(nBlack);
		}
		else if(line[0] == 's') {
			splitString(lineDiv, (char*)line.c_str(), "\n");
			int answers = countChars(lineDiv[1], '_');
			Card nBlack(lineDiv[1], 'b', answers);
			blackDeck.push_back(nBlack);
		}
	}
	
	while (getline(whiteCardReader, line)) {
		Card nWhite((char*)line.c_str(), 'w', 0);
		whiteDeck.push_back(nWhite);
	}
	FD_ZERO(&recvSockSet);
	
	FD_SET(serverSock, &recvSockSet);
	maxDesc = max(maxDesc, serverSock);
	
	//shuffle decks
	
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
			
			FD_SET(clientSock, &recvSockSet);
			maxDesc = max(maxDesc, clientSock);
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
