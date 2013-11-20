
#include "Player.h"
#include "Card.h"

Player::Player() {
	name = "";
	socket = 0;
	score = 0;
}

Player::Player(int sock, char* nam) {
  name = nam;
  socket = s;
  score = 0;
}

int Player::getSocket() {
	return socket;
}

int Player::getScore() {
	return score;
}

void Player::setSocket(int sock) {
	socket = sock;
}

void Player::addPoint() {
	score++;
}

void Player::addWinningCard(Card newCard) {
	winners.push_back(newCard);
}

void Player::addCard(Card newCard) {
	hand.push_back(newCard);
}

Card takeCard(int index) {
	Card temp = hand[index];
	hand.erase(hand.begin()+index);
	return temp;
}

std::vector<Card> getHand() {
	return hand;
}

std::vector<Card> getWinners() {
	return winners;
}