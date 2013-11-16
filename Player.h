
#ifndef Player_H
#define Player_H

#include "Card.h"
#include <vector>

class Player {

 public:
  int socket;
  std::vector<Card> hand;
  
  Player(int s);
}
