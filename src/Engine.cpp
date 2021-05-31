#include "main.hpp"


Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP), fovRadius(10),
						    screenWidth(screenWidth), screenHeight(screenHeight) {
  TCODConsole::initRoot(screenWidth, screenHeight, "foik", true);
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

void Engine::init() {
  int nbFloors = TCODRandom::getInstance()->getInt(4,8);

  while(nbFloors > 0) {
    Floor *floor = new Floor();
    floor->map = new Map(screenWidth - 20, screenHeight - 7, floor);
    floor->map->init(true);
    floors.push(floor);
    nbFloors --;
  }

  currentFloor = floors.get(0);
  currentFloorIndex = 0;
  
  int floorIndex = 0;
  for(auto floor:floors) {
    Floor *nextFloor;
    if(floorIndex == floors.size() - 1) break;

    if(floorIndex == 0) {
      Actor *actor = new Actor(floor, 0, 0, 'E', "entrance", TCODColor::lightHan);
      floor->map->addUpStairs(actor);
    }
    
    nextFloor = floors.get(floorIndex + 1);

    Portal *portal = new Portal(floorIndex + 1,
				nextFloor->map->entryx,
				nextFloor->map->entryy);
    Actor *actor = new Actor(floor,0, 0, '>', "downstairs", TCODColor::darkSepia);
    actor->portal = portal;
    actor->fovOnly = false;
    actor->blocks = false;
      
    std::pair<int, int> downStairsPos = floor->map->addDownStairs(actor);
      
    portal = new Portal(floorIndex,
			downStairsPos.first,
			downStairsPos.second);
    actor = new Actor(nextFloor, 0, 0, '<', "upstairs", TCODColor::darkSepia);
    actor->portal = portal;
    actor->fovOnly = false;
    actor->blocks = false;
    nextFloor->map->addUpStairs(actor);
    floorIndex++;
  }

  
  player = new Actor(currentFloor, 40, 25, '@', "player", TCODColor::white);
  player->destructible = new PlayerDestructible(30, 2, "your corpse");
  player->attacker = new Attacker(5);
  player->ai = new PlayerAi();
  player->container = new Container(26);

  player->x = currentFloor->map->entryx;
  player->y = currentFloor->map->entryy;

  currentFloor->actors.push(player);

  gui->message(TCODColor::green, "Welcome to dungeon, master!");
  gameStatus = STARTUP;
}

void Engine::update() {
  if(gameStatus == STARTUP)
    currentFloor->map->computeFov();

  gameStatus = NEW_TURN;
  
  TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);
  if(lastKey.vk == TCODK_ESCAPE) {
    save();
    load(true);
    currentFloor->map->computeFov();
  }

  if(lastKey.c == '[') {
    currentFloorIndex = std::max(currentFloorIndex - 1, 0);
    currentFloor = floors.get(currentFloorIndex);
  }
  
  if(lastKey.c == ']') {
    currentFloorIndex = std::min(currentFloorIndex + 1, floors.size() - 1);
    currentFloor = floors.get(currentFloorIndex);
  }

  if(gameStatus == NEW_TURN) {
    for(auto floor: floors) {
      for(auto actor: floor->actors)
	actor->update();
    }
  }
}

void Engine::render() {
  TCODConsole::root->clear();
  currentFloor->map->render();
  
  for (auto actor: currentFloor->actors) {
      actor->render();
  }

  gui->render();
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
    TCODConsole::setFullscreen(false);
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
    
    player = new Actor(NULL, 0,0,0,NULL,TCODColor::white);
    player->load(saver);

    gui->load(saver);
    gameStatus = STARTUP;
  } else {
    term();
    init();
  }
  
}

int Engine::countMonsters() const {
  int nbMonsters = 0;
  for(auto floor: floors) {
    for(auto actor: floor->actors) {
      if(actor != player && actor->destructible && !actor->destructible->isDead())
	nbMonsters++;
    }
  }
  return nbMonsters;
}
