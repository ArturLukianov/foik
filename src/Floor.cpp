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

Actor *Floor::getEnemy(int x, int y) const {
  for(auto actor: actors) {
    if(actor->x == x && actor->y == y && actor->destructible && !actor->destructible->isDead() &&
       actor->isEnemy)
      return actor;
  }
  return NULL;
}

Actor * Floor::getClosestMonster(int x, int y, float range) const {
  Actor *closest=NULL;
  float bestDistance=1E6f;
  for(auto actor : actors) {
    if(!actor->isEnemy && actor->destructible && !actor->destructible->isDead()) {
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

Actor *Floor::getEnemyInFov(int x, int y) const {
  map->computeFov(x, y);
  for(auto actor: actors) {
    if(actor->isEnemy && actor->destructible && !actor->destructible->isDead() && map->isInFov(actor->x, actor->y))
      return actor;
  }
  return NULL;
}

void Floor::addCore(Actor *core) {
  static const int moveMask[4][2] = {
    {0, 1},
    {1, 0},
    {0, -1},
    {-1, 0}
  };
  std::queue<std::pair<int, int>> q;
  int width = map->width;
  int height = map->height;

  bool used[width][height];
  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      used[x][y] = false;
    }
  }

  used[map->entryx][map->entryy] = true;
  q.push({map->entryx, map->entryy});

  int lastx, lasty;

  while(q.size() > 0) {
    std::pair<int, int> node = q.front();
    q.pop();
    int nodex = node.first;
    int nodey = node.second;
    lastx = nodex;
    lasty = nodey;
    
    for(int i = 0; i < 4; i++) {
      int nextx = nodex + moveMask[i][0];
      int nexty = nodey + moveMask[i][1];
      if(!used[nextx][nexty] && !map->isWall(nextx, nexty)) {
	used[nextx][nexty] = true;
	q.push({nextx, nexty});
      }
    }
  }

  core->x = lastx;
  core->y = lasty;

  actors.push(core);
}
