
/*
 * Card class header file
 *
 */

#ifndef Card_H
#define Card_H

#include <string.h>

class Card {

 public:
  string content;
  char type;
  int numOfAnswers;

  Card(string c, char t, int n);
  Card(string c);
}
