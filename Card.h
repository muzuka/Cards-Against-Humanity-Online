
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
	std::string content;	// card content
	char type;				// 'b' = black card, 'w' = white card
	int numOfAnswers;		// number of required white cards
	Player *owner;			// owner

	Card();
	Card(std::string, char, int, Player);
	Card(std::string, Player);
};