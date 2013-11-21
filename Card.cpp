
#include "Card.h"
#include "Player.h"

Card::Card() {
	content = (char*)"";
	type = ' ';
	numOfAnswers = 0;
	owner = NULL;
}

Card::Card(char* c, char t, int n, Player p) {
	content = c;
	type = t;
	numOfAnswers = n;
	owner = &p;
}

Card::Card(char* c, char t, int n) {
	content = c;
	type = t;
	numOfAnswers = n;
	owner = NULL;
}

Card::Card(char* c, Player p) {
	content = c;
	type = 'w';
	numOfAnswers = 0;
	owner = &p;
}
