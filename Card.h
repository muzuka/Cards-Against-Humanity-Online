
/*
 *	Card class header file
 *
 *	Written by: Sean Brown
 *	UCID:  10062604
 *
 *	Version 1.0
 *
 *	Features:
 *		+ contains contents, type of card, # of answers, owner
 */

#pragma once

#include <string>

class Player;

class Card {

  public:
	std::string content;			// card content
	char type;				// 'b' = black card, 'w' = white card
	int numOfAnswers;		// number of required white cards
	Player *owner;			// owner

	Card();
	Card(std::string, char, int, Player);
	Card(std::string, char, int);
	Card(std::string, Player);
	
	void print();
	std::string toString();
};