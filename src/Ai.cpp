#include "main.hpp"


static const int TRACKING_TURNS = 3;


Ai * Ai::create(Saver &saver) {
  AiType type = (AiType) saver.getInt();
  Ai *ai = NULL;
  switch(type) {
  case PLAYER: ai = new PlayerAi(); break;
  case MONSTER: ai = new MonsterAi(); break;
  case CONFUSED_MONSTER: ai = new ConfusedMonsterAi(0, NULL); break;
  }
  ai->load(saver);
  return ai;
}

PlayerAi::PlayerAi() : xpLevel(1), state(EXPLORE) {}

const int LEVEL_UP_BASE = 5;
const int LEVEL_UP_FACTOR = 10;

int PlayerAi::getNextLevelXp() {
  return LEVEL_UP_BASE + LEVEL_UP_FACTOR * xpLevel;
}

void PlayerAi::update(Actor *owner) {
  int levelUpXp = getNextLevelXp();
  if(owner->destructible->xp >= levelUpXp) {
    xpLevel++;
    owner->destructible->xp -= levelUpXp;
    engine.gui->message(TCODColor::green, "Your battle skills grow stronger! You reached level %d!", xpLevel);
    /*    engine.gui->menu.clear();
    
    engine.gui->menu.addItem(Menu::CONSTITUTION,"Constitution (+20HP)");
    engine.gui->menu.addItem(Menu::STRENGTH,"Strength (+1 attack)");
    engine.gui->menu.addItem(Menu::AGILITY,"Agility (+1 defense)");*/

    int choice = TCODRandom::getInstance()->getInt(0, 2); // engine.gui->menu.pick(Menu::PAUSE);
    switch(choice) {
    case 0:
      owner->destructible->maxHp += 20;
      owner->destructible->hp += 20;
      engine.gui->message(TCODColor::green, "You feel yourself tougher", xpLevel);
      break;
    case 1:
      owner->attacker->power += 1;
      engine.gui->message(TCODColor::green, "You feel yourself stronger", xpLevel);
      break;
    case 2:
      owner->destructible->defense += 1;
      engine.gui->message(TCODColor::green, "You feel agile", xpLevel);
      break;
    default: break;
    }
  }
  
  if(owner->destructible && owner->destructible->isDead()) return;

  for (int x = 0; x < engine.currentFloor->map->width; x++) {
    for (int y = 0; y < engine.currentFloor->map->height; y++) {
      engine.currentFloor->map->isInFov(x, y);
    }
  }

  int dx = 0, dy = 0;

  if(owner->destructible && owner->destructible->hp <= owner->destructible->maxHp - 4) {
    for(auto item: owner->container->inventory) {
      if(!strcmp(item->name, "health potion")) {
	item->pickable->use(item, owner);
	break;
      }
    }
  }
  
  if(isMonsterInFov(owner)) {
    state = ATTACK_MONSTER;
  }
    
  if(state == ATTACK_MONSTER) {
    for(auto item: owner->container->inventory) {
      if(!strcmp(item->name, "lightning bolt")) {
	item->pickable->use(item, owner);
	break;
      }
    }
    if(!getAttackMove(owner, &dx, &dy)) {
      state = EXPLORE;
    }
  } else {
    if(owner->container && owner->container->inventory.size() < owner->container->size && isItemInFov(owner))
      state = PICK_ITEM;
  }

  if (state == PICK_ITEM) {
    if(!getPickMove(owner, &dx, &dy)) {
      state = EXPLORE;
    } else {
      if(dx == 0 && dy == 0) {
	pickItemFromTile(owner);
	state = EXPLORE;
      }
    }
  }
  
  if(state == EXPLORE) {
    if(!getExploreMove(owner, &dx, &dy)) {
      state = NEXT_FLOOR;
    }
  }

  if(state == NEXT_FLOOR) {
    getPortalMove(owner, &dx, &dy);
    if(dx == 0 && dy == 0) {
      Actor *actor = engine.getPortal(owner->x, owner->y);
      if(actor) {
	actor->portal->warp(actor, owner);
	engine.currentFloor->map->computeFov();
      }
      state = EXPLORE;
    }
  }
  
  switch(engine.lastKey.vk) {
  case TCODK_UP : dy = -1; break;
  case TCODK_DOWN : dy = 1; break;
  case TCODK_LEFT : dx = -1; break;
  case TCODK_RIGHT : dx = 1; break;
  case TCODK_CHAR : handleActionKey(owner, engine.lastKey.c); break;
  default:
    break;
  }

  if(dx != 0 || dy != 0) {
    engine.gameStatus = Engine::NEW_TURN;
    if(moveOrAttack(owner, owner->x+dx, owner->y+dy))
      engine.currentFloor->map->computeFov();
  }
}

bool PlayerAi::isItemInFov(Actor *owner) {
  bool found=false;
  for(auto actor : engine.currentFloor->actors) {
    if(actor->pickable && engine.currentFloor->map->isInFov(actor->x, actor->y)) {
      found = true;
      break;
    }
  }
  return found;
}

bool PlayerAi::isMonsterInFov(Actor *owner) {
  for(auto actor : engine.currentFloor->actors) {
    if(actor->destructible && !actor->destructible->isDead() && engine.currentFloor->map->isInFov(actor->x, actor->y)) {
      return true;
    }
  }
  return false;
}

bool PlayerAi::isMonsterOnTile(Actor *owner, int x, int y) {
  for(auto actor : engine.currentFloor->actors) {
    if(actor->destructible && !actor->destructible->isDead() && actor->x == x && actor->y == y) {
      return true;
      break;
    }
  }
  return false;
}

bool PlayerAi::getExploreMove(Actor *owner, int *dx, int *dy) {
  static const int moveMask[4][2] = {
    {0, 1},
    {1, 0},
    {0, -1},
    {-1, 0}
  };
  std::queue<std::pair<int, int>> q;
  int width = engine.currentFloor->map->width;
  int height = engine.currentFloor->map->height;
  
  std::pair<int, int> path[width][height];
  bool used[width][height];
  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      used[x][y] = false;
    }
  }

  int targetx, targety;

  used[owner->x][owner->y] = true;
  path[owner->x][owner->y] = {owner->x, owner->y};
  q.push({owner->x, owner->y});
  bool found = false;
  while(q.size() > 0) {
    std::pair<int, int> node = q.front();
    q.pop();
    int nodex = node.first;
    int nodey = node.second;

    if(!engine.currentFloor->map->isExplored(nodex, nodey)) {
      targetx = nodex;
      targety = nodey;
      found = true;
      break;
    }
    
    for(int i = 0; i < 4; i++) {
      int nextx = nodex + moveMask[i][0];
      int nexty = nodey + moveMask[i][1];
      if(!used[nextx][nexty] && engine.currentFloor->map->canWalk(nextx, nexty)) {
	used[nextx][nexty] = true;
	path[nextx][nexty] = {nodex, nodey};
	q.push({nextx, nexty});
      }
    }
  }
  if(found) {
    do {
      (*dx) = targetx - owner->x;
      (*dy) = targety - owner->y;
      std::pair<int, int> nextNode = path[targetx][targety];
      targetx = nextNode.first;
      targety = nextNode.second;
    } while(path[targetx][targety].first != targetx || path[targetx][targety].second != targety);
  }
  return found;
}

void PlayerAi::getItemInFovPos(Actor *owner, int *x, int *y) {
  for(auto actor : engine.currentFloor->actors) {
    if(actor->pickable && engine.currentFloor->map->isInFov(actor->x, actor->y)) {
      (*x) = actor->x;
      (*y) = actor->y;
      break;
    }
  }
}

bool PlayerAi::getPickMove(Actor *owner, int *dx, int *dy) {
  static const int moveMask[4][2] = {
    {0, 1},
    {1, 0},
    {0, -1},
    {-1, 0}
  };
  std::queue<std::pair<int, int>> q;
  int width = engine.currentFloor->map->width;
  int height = engine.currentFloor->map->height;
  
  std::pair<int, int> path[width][height];
  bool used[width][height];
  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      used[x][y] = false;
    }
  }

  int targetx, targety;

  getItemInFovPos(owner, &targetx, &targety);

  used[owner->x][owner->y] = true;
  path[owner->x][owner->y] = {owner->x, owner->y};
  q.push({owner->x, owner->y});
  bool found = false;
  while(q.size() > 0) {
    std::pair<int, int> node = q.front();
    q.pop();
    int nodex = node.first;
    int nodey = node.second;

    if(nodex == targetx && nodey == targety) {
      found = true;
      break;
    }
    
    for(int i = 0; i < 4; i++) {
      int nextx = nodex + moveMask[i][0];
      int nexty = nodey + moveMask[i][1];
      if(engine.currentFloor->map->isInFov(nextx, nexty) && !used[nextx][nexty] && engine.currentFloor->map->canWalk(nextx, nexty)) {
	used[nextx][nexty] = true;
	path[nextx][nexty] = {nodex, nodey};
	q.push({nextx, nexty});
      }
    }
  }
  if(found) {
    do {
      (*dx) = targetx - owner->x;
      (*dy) = targety - owner->y;
      std::pair<int, int> nextNode = path[targetx][targety];
      targetx = nextNode.first;
      targety = nextNode.second;
    } while(path[targetx][targety].first != targetx || path[targetx][targety].second != targety);
  }
  return found;
}


void PlayerAi::getMonsterInFovPos(Actor *owner, int *x, int *y) {
  for(auto actor : engine.currentFloor->actors) {
    if(actor->destructible && !actor->destructible->isDead() && engine.currentFloor->map->isInFov(actor->x, actor->y)) {
      (*x) = actor->x;
      (*y) = actor->y;
      break;
    }
  }
}

bool PlayerAi::getAttackMove(Actor *owner, int *dx, int *dy) {
  static const int moveMask[4][2] = {
    {0, 1},
    {1, 0},
    {0, -1},
    {-1, 0}
  };
  std::queue<std::pair<int, int>> q;
  int width = engine.currentFloor->map->width;
  int height = engine.currentFloor->map->height;
  
  std::pair<int, int> path[width][height];
  bool used[width][height];
  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      used[x][y] = false;
    }
  }

  int targetx, targety;

  used[owner->x][owner->y] = true;
  path[owner->x][owner->y] = {owner->x, owner->y};
  q.push({owner->x, owner->y});
  bool found = false;
  while(q.size() > 0) {
    std::pair<int, int> node = q.front();
    q.pop();
    int nodex = node.first;
    int nodey = node.second;

    if(isMonsterOnTile(owner, nodex, nodey)) {
      found = true;
      targetx = nodex;
      targety = nodey;
      break;
    }
    
    for(int i = 0; i < 4; i++) {
      int nextx = nodex + moveMask[i][0];
      int nexty = nodey + moveMask[i][1];
      if(engine.currentFloor->map->isInFov(nextx, nexty) && !used[nextx][nexty] && (engine.currentFloor->map->canWalk(nextx, nexty) || (isMonsterOnTile(owner, nextx, nexty)))) {
	used[nextx][nexty] = true;
	path[nextx][nexty] = {nodex, nodey};
	q.push({nextx, nexty});
      }
    }
  }
  if(found) {
    do {
      (*dx) = targetx - owner->x;
      (*dy) = targety - owner->y;
      std::pair<int, int> nextNode = path[targetx][targety];
      targetx = nextNode.first;
      targety = nextNode.second;
    } while(path[targetx][targety].first != targetx || path[targetx][targety].second != targety);
  }
  return found;
}


bool PlayerAi::moveOrAttack(Actor *owner, int targetx, int targety) {
  if(engine.currentFloor->map->isWall(targetx, targety)) return false;

  for(auto actor : engine.currentFloor->actors) {
    if(actor->destructible && !actor->destructible->isDead() &&
       actor->x == targetx && actor->y == targety) {
      owner->attacker->attack(owner, actor);
      return false;
    }
  }

  for(auto actor : engine.currentFloor->actors) {
    bool corpseOrItem = ((actor->destructible && actor->destructible->isDead()) || (actor->pickable));
    if ( corpseOrItem && actor->x == targetx && actor->y == targety ) {
      engine.gui->message(TCODColor::yellow, "There's a %s here",actor->name);
    }
  }
  owner->x = targetx;
  owner->y = targety;
  return true;
}

void PlayerAi::pickItemFromTile(Actor *owner) {
  bool found=false;
  for(auto actor : engine.currentFloor->actors) {
    if(actor->pickable && actor->x == owner->x && actor->y == owner->y) {
      if(actor->pickable->pick(actor, owner)) {
	found = true;
	engine.gui->message(TCODColor::lightGrey, "You picked up %s", actor->name);
	break;
      } else if (!found) {
	found = true;
	engine.gui->message(TCODColor::red, "Your inventory is full");
      }
    }
  }
  if(!found) {
    engine.gui->message(TCODColor::lightGrey, "There is nothing to pick up");
  }
  engine.gameStatus = Engine::NEW_TURN;
}

bool PlayerAi::getNextFloorPortal(Actor *owner, int *x, int *y) {
  for(auto actor : engine.currentFloor->actors) {
    if(actor->portal && actor->ch == '>') {
      (*x) = actor->x;
      (*y) = actor->y;
      return true;
    }
  }
  return false;
}

bool PlayerAi::getPortalMove(Actor *owner, int *dx, int *dy) {
  static const int moveMask[4][2] = {
    {0, 1},
    {1, 0},
    {0, -1},
    {-1, 0}
  };
  std::queue<std::pair<int, int>> q;
  int width = engine.currentFloor->map->width;
  int height = engine.currentFloor->map->height;
  
  std::pair<int, int> path[width][height];
  bool used[width][height];
  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      used[x][y] = false;
    }
  }

  int targetx, targety;
  if(!getNextFloorPortal(owner, &targetx, &targety))
    return false;

  used[owner->x][owner->y] = true;
  path[owner->x][owner->y] = {owner->x, owner->y};
  q.push({owner->x, owner->y});
  bool found = false;
  while(q.size() > 0) {
    std::pair<int, int> node = q.front();
    q.pop();
    int nodex = node.first;
    int nodey = node.second;

    if(targetx == nodex && targety == nodey) {
      found = true;
      break;
    }
    
    for(int i = 0; i < 4; i++) {
      int nextx = nodex + moveMask[i][0];
      int nexty = nodey + moveMask[i][1];
      if(!used[nextx][nexty] && engine.currentFloor->map->canWalk(nextx, nexty)) {
	used[nextx][nexty] = true;
	path[nextx][nexty] = {nodex, nodey};
	q.push({nextx, nexty});
      }
    }
  }
  if(found) {
    do {
      (*dx) = targetx - owner->x;
      (*dy) = targety - owner->y;
      std::pair<int, int> nextNode = path[targetx][targety];
      targetx = nextNode.first;
      targety = nextNode.second;
    } while(path[targetx][targety].first != targetx || path[targetx][targety].second != targety);
  }
  return found;
}

void PlayerAi::handleActionKey(Actor *owner, int ascii) {
  switch(ascii) {
  case 'g' :
    {
      pickItemFromTile(owner);
    }
    break;
  case 'i' :
    {
      Actor *actor = chooseFromInventory(owner);
      if(actor) {
	actor->pickable->use(actor, owner);
	engine.gameStatus = Engine::NEW_TURN;
      }
    }
    break;
  case 'd':
    {
      Actor *actor = chooseFromInventory(owner);
      if(actor) {
	actor->pickable->drop(actor, owner);
	engine.gameStatus = Engine::NEW_TURN;
      }
    }
    break;
  case 'e':
    {
      Actor *actor = engine.getPortal(owner->x, owner->y);
      if(actor) {
	actor->portal->warp(actor, owner);
	engine.currentFloor->map->computeFov();
      }
    }
  }
}

void PlayerAi::save(Saver &saver) {
  saver.putInt(PLAYER);
  saver.putInt(xpLevel);
}

void PlayerAi::load(Saver &saver) {
  xpLevel = saver.getInt();
}


MonsterAi::MonsterAi() : moveCount(0) {}


void MonsterAi::update(Actor *owner) {
  if(owner->destructible && owner->destructible->isDead()) return;

  if(engine.currentFloor->map->isInFov(owner->x, owner->y)) {
    moveCount = TRACKING_TURNS;
  } else {
    moveCount--;
  }

  if(moveCount > 0) {
    moveOrAttack(owner, engine.player->x, engine.player->y);
  }
}

bool MonsterAi::moveOrAttack(Actor *owner, int targetx, int targety) {
  int dx = targetx - owner->x;
  int dy = targety - owner->y;

  int stepdx = (dx > 0 ? 1 : -1);
  int stepdy = (dy > 0 ? 1 : -1);

  float distance = sqrtf(dx*dx+dy*dy);

  if(distance >= 2) {
    dx = (int)(round(dx/distance));
    dy = (int)(round(dy/distance));

    if(engine.currentFloor->map->canWalk(owner->x + dx, owner->y + dy)) {
      owner->x += dx;
      owner->y += dy;
    } else if(engine.currentFloor->map->canWalk(owner->x + stepdx, owner->y)) {
      owner->x += stepdx;
    } else if(engine.currentFloor->map->canWalk(owner->x, owner->y + stepdy)) {
      owner->y += stepdy;
    }
  } else if (owner->attacker) {
    owner->attacker->attack(owner, engine.player);
  }
}

void MonsterAi::save(Saver &saver) {
  saver.putInt(MONSTER);
  saver.putInt(moveCount);
}

void MonsterAi::load(Saver &saver) {
  moveCount = saver.getInt();
}


Actor *PlayerAi::chooseFromInventory(Actor *owner) {
  static const int INVENTORY_WIDTH=50;
  static const int INVENTORY_HEIGHT=28;
  static TCODConsole con(INVENTORY_WIDTH,INVENTORY_HEIGHT);
   
  con.setDefaultForeground(TCODColor(200,180,50));
  con.printFrame(0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,true,
		 TCOD_BKGND_DEFAULT,"inventory");

  con.setDefaultForeground(TCODColor::white);
  int shortcut = 'a';
  int y = 1;
  for(auto actor : owner->container->inventory) {
    con.print(2, y, "(%c) %s", shortcut, actor->name);
    y++;
    shortcut++;
  }

  TCODConsole::blit(&con, 0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,
		    TCODConsole::root, engine.screenWidth/2 - INVENTORY_WIDTH/2,
		    engine.screenHeight/2-INVENTORY_HEIGHT/2);
  TCODConsole::flush();

  TCOD_key_t key;
  TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);

  if(key.vk == TCODK_CHAR) {
    int actorIndex = key.c - 'a';
    if(actorIndex >= 0 && actorIndex < owner->container->inventory.size())
      return owner->container->inventory.get(actorIndex);
  }
  return NULL;
}


ConfusedMonsterAi::ConfusedMonsterAi(int nbTurns, Ai *oldAi) : nbTurns(nbTurns), oldAi(oldAi) {
}

void ConfusedMonsterAi::update(Actor *owner) {
  TCODRandom *rng = TCODRandom::getInstance();
  int dx = rng->getInt(-1, 1);
  int dy = rng->getInt(-1, 1);

  int destx = owner->x + dx;
  int desty = owner->y + dy;
  if(engine.currentFloor->map->canWalk(destx, desty)) {
    owner->x = destx;
    owner->y = desty;
  } else {
    Actor *actor = engine.getActor(destx, desty);
    if(actor) {
      owner->attacker->attack(owner, actor);
    }
  }

  nbTurns--;
  if(nbTurns == 0) {
    owner->ai = oldAi;
    delete this;
  }
}

void ConfusedMonsterAi::load(Saver &saver) {
  nbTurns = saver.getInt();
  oldAi = Ai::create(saver);
}

void ConfusedMonsterAi::save(Saver &saver) {
  saver.putInt(CONFUSED_MONSTER);
  saver.putInt(nbTurns);
  oldAi->save(saver);
}
