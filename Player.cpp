
#include <cstring>
#include "Player.h"
#include "Card.h"

Player::Player() {
	name = (char*)"";
	socket = 0;
	score = 0;
}

Player::Player(int sock) {
  name = (char*)"";
  socket = sock;
  score = 0;
}

char* Player::getName() {
	return name;
}

void Player::setName(char* newName) {
	name = newName;
}

int Player::getSocket() {
	return socket;
}

void Player::setSocket(int sock) {
	socket = sock;
}

int Player::getScore() {
	return score;
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

Card Player::takeCard(int index) {
	Card temp = hand[index];
	hand.erase(hand.begin()+index);
	return temp;
}

bool Player::isEqual(Player p) {
	if(strcmp(name, p.getName()) == 0) {
		return true;
	}
	else {
		return false;
	}

}

bool Player::isEqual(Player p, Player q) {
	if(strcmp(p.getName(), q.getName()) == 0) {
		return true;
	}
	else {
		return false;
	}
}

std::vector<Card> Player::getHand() {
	return hand;
}

std::vector<Card> Player::getWinners() {
	return winners;
}