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

PlayerAi::PlayerAi() : xpLevel(1) {}

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
    engine.gui->menu.clear();
    
    engine.gui->menu.addItem(Menu::CONSTITUTION,"Constitution (+20HP)");
    engine.gui->menu.addItem(Menu::STRENGTH,"Strength (+1 attack)");
    engine.gui->menu.addItem(Menu::AGILITY,"Agility (+1 defense)");

    Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::PAUSE);
    switch(menuItem) {
    case Menu::CONSTITUTION:
      owner->destructible->maxHp += 20;
      owner->destructible->hp += 20;
      break;
    case Menu::STRENGTH:
      owner->attacker->power += 1;
      break;
    case Menu::AGILITY:
      owner->destructible->defense += 1;
      break;
    default: break;
    }
  }
  
  if(owner->destructible && owner->destructible->isDead()) return;

  int dx = 0, dy = 0;
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

void PlayerAi::handleActionKey(Actor *owner, int ascii) {
  switch(ascii) {
  case 'g' :
    {
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
