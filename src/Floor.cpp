#include "main.hpp"

Floor::Floor() : map(NULL) {}

Floor::~Floor() {
  if (map) delete map;
  actors.clearAndDelete();
}

void Floor::save(Saver &saver) {
  saver.putInt(map->width);
  saver.putInt(map->height);
  map->save(saver);
  saver.putInt(actors.size());
  for(auto actor : actors) {
    actor->save(saver);
  }
}

void Floor::load(Saver &saver) {
  int width = saver.getInt();
  int height = saver.getInt();
  map = new Map(width, height, this);
  
  map->load(saver);

  int nbActors = saver.getInt();
  actors.clearAndDelete();
  while(nbActors > 0) {
    Actor *actor = new Actor(this,0,0,0,NULL,TCODColor::white);
    actor->load(saver);
    actors.push(actor);
    nbActors--;
  }
}

void Floor::sendToBack(Actor *actor) {
  actors.remove(actor);
  actors.insertBefore(actor, 0);
}


Actor *Floor::getActor(int x, int y) const {
  for(auto actor: actors) {
    if(actor->x == x && actor->y == y && actor->destructible && !actor->destructible->isDead())
      return actor;
  }
  return NULL;
}

Actor * Floor::getClosestMonster(int x, int y, float range) const {
  Actor *closest=NULL;
  float bestDistance=1E6f;
  for(auto actor : actors) {
    if(actor != engine.player && actor->destructible && !actor->destructible->isDead()) {
      float distance = actor->getDistance(x, y);
      if(distance < bestDistance && (distance <= range || range == 0.0f)) {
	bestDistance = distance;
	closest = actor;
      }
    }
  }
  return closest;
}

Actor *Floor::getPortal(int x, int y) const {
  for (auto actor : actors) {
    if(actor->portal && actor->x == x && actor->y == y)
      return actor;
  }
  return NULL;
}
