
/*
 *	Implementation of Card.h
 *	
 *	Written by: Sean Brown
 *	UCID: 10062604
 *
 *	Version 1.0
 *
 *	Features:
 *		+ All needed features have been finished
 */

#include <cstring>
#include <string>
#include <stdio.h>
#include "Card.h"
#include "Player.h"

// basic constructor
Card::Card() {
	content = "";
	type = ' ';
	numOfAnswers = 0;
	owner = "";
}

// complete constructor
Card::Card(std::string c, char t, int n, std::string p) {
	content = c;
	type = t;
	numOfAnswers = n;
	owner = p;
}

// complete constructor
Card::Card(std::string c, char t, int n) {
	content = c;
	type = t;
	numOfAnswers = n;
	owner = "";
}

// quick white card constructor
Card::Card(std::string c, std::string p) {
	content = c;
	type = 'w';
	numOfAnswers = 0;
	owner = p;
}

void Card::print() {
	printf("%s\n", content.c_str());
}