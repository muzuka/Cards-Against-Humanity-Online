
/*
 * Card class header file
 *
 */

#ifndef Card_H
#define Card_H

#include <string>
#include <Player.h>

class Card {

 public:
  string content;
  char type;
  int numOfAnswers;
  Player owner;

  Card(string c, char t, int n, Player p);
  Card(string c, Player p);
}
