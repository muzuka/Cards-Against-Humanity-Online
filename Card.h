
/*
 * Card class header file
 *
 */

#pragma once


#include <string.h>
//#include "Player.h"

class Player;

class Card {

  public:
	std::string content;
	char type;
	int numOfAnswers;
	Player *owner;

	Card();
	Card(std::string, char, int, Player);
	Card(std::string, Player);
};