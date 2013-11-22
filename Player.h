
#pragma once

#include <vector>
#include <string>

class Card;

class Player {

 private:
	std::string name;
	int socket;
	int score;
	std::vector<Card> hand;
	std::vector<Card> winners;
  
 public:
	Player();
	Player(int);
	Player(std::string, int);
	
	std::string getName();
	void setName(std::string);
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