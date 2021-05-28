#include "main.hpp"


Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP), fovRadius(10),
						    screenWidth(screenWidth), screenHeight(screenHeight) {
  TCODConsole::initRoot(screenWidth, screenHeight, "foik", false);
  gui = new Gui();
}

Engine::~Engine() {
  term();
  delete gui;
}

void Engine::term() {
  floors.clearAndDelete();
  gui->clear();
}  

int Engine::getFloorIndex(Floor *needle) const {
  int index = 0;

  for(auto floor:floors) {
    if(floor == needle)
      return index;
    index++;
  }
  
  return -1;
}

Actor *Engine::getPortal(int x, int y) const {
  for (auto actor : engine.currentFloor->actors) {
    if(actor->portal && actor->x == x && actor->y == y)
      return actor;
  }
  return NULL;
}

void Engine::init() {
  player = new Actor(40, 25, '@', "player", TCODColor::white);
  player->destructible = new PlayerDestructible(30, 2, "your corpse");
  player->attacker = new Attacker(5);
  player->ai = new PlayerAi();
  player->container = new Container(26);

  int nbFloors = TCODRandom::getInstance()->getInt(5, 100);

  while(nbFloors > 0) {
    Floor *floor = new Floor();
    floor->map = new Map(screenWidth, screenHeight - 7, floor);
    floor->map->init(true);
    floors.push(floor);
    nbFloors --;
  }

  currentFloor = floors.get(0);
  int floorIndex = 0;
  for(auto floor:floors) {
    Floor *nextFloor;
    if(floorIndex != floors.size() - 1) {
      nextFloor = floors.get(floorIndex + 1);

      Portal *portal = new Portal(floorIndex + 1,
				  nextFloor->map->entryx,
				  nextFloor->map->entryy);
      Actor *actor = new Actor(0, 0, '>', "downstairs", TCODColor::darkSepia);
      actor->portal = portal;
      actor->fovOnly = false;
      actor->blocks = false;
      floor->map->addPortal(actor);
    }

    if(floorIndex != 0) {
      nextFloor = floors.get(floorIndex - 1);


      Portal *portal = new Portal(floorIndex - 1,
				  nextFloor->map->entryx,
				  nextFloor->map->entryy);
      Actor *actor = new Actor(0, 0, '<', "upstairs", TCODColor::darkSepia);
      actor->portal = portal;
      actor->fovOnly = false;
      actor->blocks = false;
      floor->map->addPortal(actor);
    }
    floorIndex ++;
  }

  player->x = currentFloor->map->entryx;
  player->y = currentFloor->map->entryy;
  

  gui->message(TCODColor::green, "Welcome to dungeon, master!");
  gameStatus = STARTUP;
}

void Engine::update() {
  if(gameStatus == STARTUP)
    currentFloor->map->computeFov();

  gameStatus = IDLE;
  
  TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);
  if(lastKey.vk == TCODK_ESCAPE) {
    save();
    load(true);
    currentFloor->map->computeFov();
  }
  
  player->update();


  if(gameStatus == NEW_TURN) {
    for(auto actor: currentFloor->actors)
      actor->update();
  }
}

void Engine::render() {
  TCODConsole::root->clear();
  currentFloor->map->render();
  
  for (auto actor: currentFloor->actors) {
    if(currentFloor->map->isInFov(actor->x, actor->y) ||
       (currentFloor->map->isExplored(actor->x, actor->y) && !actor->fovOnly)) {

      actor->render();
    }
  }


  player->render();

  gui->render();
}

void Engine::sendToBack(Actor *actor) {
  currentFloor->actors.remove(actor);
  currentFloor->actors.insertBefore(actor, 0);
}

Actor * Engine::getClosestMonster(int x, int y, float range) const {
  Actor *closest=NULL;
  float bestDistance=1E6f;
  for(auto actor : currentFloor->actors) {
    if(actor != player && actor->destructible && !actor->destructible->isDead()) {
      float distance = actor->getDistance(x, y);
      if(distance < bestDistance && (distance <= range || range == 0.0f)) {
	bestDistance = distance;
	closest = actor;
      }
    }
  }
  return closest;
}

bool Engine::pickATile(int *x, int *y, float maxRange) {
  while(!TCODConsole::isWindowClosed()) {
    render();
    for(int cx=0; cx < currentFloor->map->width; cx++) {
      for(int cy=0; cy < currentFloor->map->height; cy++) {
	if(currentFloor->map->isInFov(cx, cy) &&
	   (maxRange == 0 || player->getDistance(cx, cy) <= maxRange) ) {
	  TCODColor col = TCODConsole::root->getCharBackground(cx,cy);
	  col = col * 1.2f;
	  TCODConsole::root->setCharBackground(cx, cy, col);
	}
      }
    }
    
    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);

    if ( currentFloor->map->isInFov(mouse.cx,mouse.cy)
	 && ( maxRange == 0 || player->getDistance(mouse.cx,mouse.cy) <= maxRange )) {
      TCODConsole::root->setCharBackground(mouse.cx,mouse.cy,TCODColor::white);
      if ( mouse.lbutton_pressed ) {
	*x=mouse.cx;
	*y=mouse.cy;
	return true;
      }
    }

    if (mouse.rbutton_pressed) {
      return false;
    }

    TCODConsole::flush();
  }
  return false;

}


Actor *Engine::getActor(int x, int y) const {
  for(auto actor: currentFloor->actors) {
    if(actor->x == x && actor->y == y && actor->destructible && !actor->destructible->isDead())
      return actor;
  }
  return NULL;
}

void Engine::save() {
  if(player->destructible->isDead()) {
    Saver::deleteFile("game.sav");
  } else {
    Saver saver;

    saver.putInt(floors.size());
    for(auto floor : floors)
      floor->save(saver);

    int currentFloorIndex = 0;
    for(auto floor : floors) {
      if(currentFloor == floor) break;
      currentFloorIndex++;
    }
    saver.putInt(currentFloorIndex);
    
    player->save(saver);

    gui->save(saver);
    saver.saveToFile("game.sav");
  }
}

void Engine::load(bool pause) {
  engine.gui->menu.clear();
  engine.gui->menu.addItem(Menu::NEW_GAME,"New game");
  if(Saver::fileExists("game.sav")) {
    engine.gui->menu.addItem(Menu::CONTINUE,"Continue");
  }
  engine.gui->menu.addItem(Menu::EXIT,"Exit");

  Menu::MenuItemCode menuItem=engine.gui->menu.pick(pause ? Menu::PAUSE : Menu::MAIN);

  if(menuItem == Menu::EXIT || menuItem == Menu::NONE) {
    exit(0);
  } else if(menuItem == Menu::CONTINUE) {
    term();
    Saver saver;
    saver.loadFromFile("game.sav");

    int nbFloors = saver.getInt();
    while(nbFloors > 0) {
      Floor *floor = new Floor();
      floor->load(saver);
      floors.push(floor);
      nbFloors--;
    }

    int currentFloorIndex = saver.getInt();
    currentFloor = floors.get(currentFloorIndex);
    
    player = new Actor(0,0,0,NULL,TCODColor::white);
    player->load(saver);

    gui->load(saver);
    gameStatus = STARTUP;
  } else {
    term();
    init();
  }
  
}
