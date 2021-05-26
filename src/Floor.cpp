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
    Actor *actor = new Actor(0,0,0,NULL,TCODColor::white);
    actor->load(saver);
    actors.push(actor);
    nbActors--;
  }
}
