
#include <string.h>
#include "Card.h"

Card::Card(string c, char t, int n) {
  content = c;
  type = t;
  numOfAnswers = n;
}

Card::Card(string c) {
  content = c;
  type = 'w';
  numOfAnswers = 0;
}
