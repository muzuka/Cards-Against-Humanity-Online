
#pragma once

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
	Player(int);
	
	char* getName();
	void setName(char*);
	int getSocket();
	void setSocket(int);
	int getScore();
	void addPoint();
	void addWinningCard(Card);
	void addCard(Card);
	Card takeCard(int);
	bool isEqual(Player);
	bool isEqual(Player, Player);
	
	std::vector<Card> getHand();
	std::vector<Card> getWinners();
	
	
};