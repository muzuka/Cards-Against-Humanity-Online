
#pragma once

//#include "Card.h"
#include <vector>

class Card;

class Player {

 private:
	char* name;
	int socket;
	int score;
	std::vector<Card> hand;
	std::vector<Card> winners;
  
 public:
	Player();
	Player(int, char*);
	
	int getSocket();
	int getScore();
	void setSocket(int sock);
	void addPoint();
	void addWinningCard(Card);
	void addCard(Card);
	Card takeCard(int);
	
	std::vector<Card> getHand();
	std::vector<Card> getWinners();
	
	
};