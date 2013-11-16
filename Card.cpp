
#include <string>
#include "Card.h"

Card::Card(string c, char t, int n, Player p) {
  content = c;
  type = t;
  numOfAnswers = n;
  owner = p;
}

Card::Card(string c, Player p) {
  content = c;
  type = 'w';
  numOfAnswers = 0;
  owner = p;
}
