#include "main.hpp"

Actor::Actor(Floor *currentFloor, int x, int y, int ch, const char *name, const TCODColor &col) :
  currentFloor(currentFloor), x(x), y(y), ch(ch), name(name), col(col), blocks(true), fovOnly(true), attacker(NULL), destructible(NULL), ai(NULL),
  pickable(NULL), container(NULL), portal(NULL), isEnemy(false) {
}

Actor::~Actor() {
  if(attacker) delete attacker;
  if(destructible) delete destructible;
  if(ai) delete ai;
  if(pickable) delete pickable;
  if(container) delete container;
}

void Actor::render() const {
  TCODConsole::root->setChar(x, y, ch);
  TCODConsole::root->setCharForeground(x, y, col);
}

void Actor::update() {
  if(ai) ai->update(this);
}

bool Actor::moveOrAttack(int x, int y) {
  if(currentFloor->map->isWall(x, y)) return false;
  for(auto actor : currentFloor->actors) {
    if(actor->x == x && actor->y == y) {
      return false;
    }
  }
  this->x = x;
  this->y = y;
  return true;
}

float Actor::getDistance(int cx, int cy) const {
  int dx = x - cx;
  int dy = y - cy;
  return sqrtf(dx * dx + dy * dy);
}

void Actor::save(Saver &saver) {
  saver.putInt(x);
  saver.putInt(y);
  saver.putInt(ch);
  saver.putColor(&col);
  saver.putString(name);
  saver.putInt(blocks);
  saver.putInt(fovOnly);
  saver.putInt(attacker != NULL);
  saver.putInt(destructible != NULL);
  saver.putInt(ai != NULL);
  saver.putInt(pickable != NULL);
  saver.putInt(container != NULL);
  saver.putInt(portal != NULL);
  if(attacker) attacker->save(saver);
  if(destructible) destructible->save(saver);
  if(ai) ai->save(saver);
  if(pickable) pickable->save(saver);
  if(container) container->save(saver);
  if(portal) portal->save(saver);
}

void Actor::load(Saver &saver) {
  x = saver.getInt();
  y = saver.getInt();
  ch = saver.getInt();
  col = saver.getColor();
  name = strdup(saver.getString());
  blocks = saver.getInt();
  fovOnly = saver.getInt();
  bool hasAttacker = saver.getInt();
  bool hasDestructible = saver.getInt();
  bool hasAi = saver.getInt();
  bool hasPickable = saver.getInt();
  bool hasContainer = saver.getInt();
  bool hasPortal = saver.getInt();


  if(hasAttacker) {
    attacker = new Attacker(0.0f);
    attacker->load(saver);
  }
  if(hasDestructible) {
    destructible = Destructible::create(saver);
  }

  if(hasAi) {
    ai = Ai::create(saver);
  }

  if(hasPickable) {
    pickable = Pickable::create(saver);
  }

  if(hasContainer) {
    container = new Container(0);
    container->load(saver);
  }

  if(hasPortal) {
    portal = new Portal(0, 0, 0);
    portal->load(saver);
  }
}
