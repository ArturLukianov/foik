#include "main.hpp"


Container::Container(int size) : size(size) {
}

Container::~Container() {
  inventory.clearAndDelete();
}

bool Container::add(Actor *actor) {
  if(size > 0 && inventory.size() >= size)
    return false;
  inventory.push(actor);
  return true;
}

void Container::remove(Actor *actor) {
  inventory.remove(actor);
}

void Container::save(Saver &saver) {
  saver.putInt(size);
  saver.putInt(inventory.size());
  for(auto actor:inventory)
    actor->save(saver);
}

void Container::load(Saver &saver) {
  size = saver.getInt();
  int nbActors = saver.getInt();

  while(nbActors > 0) {
    Actor *actor = new Actor(0, 0, 0, NULL, TCODColor::white);
    actor->load(saver);
    inventory.push(actor);
    nbActors--;
  }
}
