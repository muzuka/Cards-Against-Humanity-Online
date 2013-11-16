
#ifndef Player_H
#define Player_H

#include "Card.h"
#include <vector>

class Player {

 public:
  int socket;
  int score;
  std::vector<Card> hand;
  std::vector<Card> winners;
  
  Player(int s);
}
