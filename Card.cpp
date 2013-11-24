
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

Card::Card() {
	content = "";
	type = ' ';
	numOfAnswers = 0;
	owner = NULL;
}

Card::Card(std::string c, char t, int n, Player p) {
	content = c;
	type = t;
	numOfAnswers = n;
	owner = &p;
}

Card::Card(std::string c, char t, int n) {
	content = c;
	type = t;
	numOfAnswers = n;
	owner = NULL;
}

Card::Card(std::string c, Player p) {
	content = c;
	type = 'w';
	numOfAnswers = 0;
	owner = &p;
}

void Card::print() {
	printf("%s\n", content.c_str());
}

std::string Card::toString() {
	char* name = strcat((char*)owner->getName().c_str(), "\n");
	std::string temp(strcat(name, content.c_str()));
	return temp;
}