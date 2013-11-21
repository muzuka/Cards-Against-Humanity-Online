
/*
 * Card class header file
 *
 */

#pragma once

class Player;

class Card {

  public:
	char* content;			// card content
	char type;				// 'b' = black card, 'w' = white card
	int numOfAnswers;		// number of required white cards
	Player *owner;			// owner

	Card();
	Card(char*, char, int, Player);
	Card(char*, char, int);
	Card(char*, Player);
};