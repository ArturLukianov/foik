#include "main.hpp"


ActorTarget::ActorTarget(Actor *actor) :
  actor(actor) {}


int ActorTarget::getX() {
  return actor->x;
}

int ActorTarget::getY() {
  return actor->y;
}

bool ActorTarget::isDead() {
  return actor->destructible && actor->destructible->isDead();
}

Floor *ActorTarget::getFloor() {
  return actor->currentFloor;
}
