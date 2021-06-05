#include "main.hpp"


static const int turnDelay = 100;

static int intrusionTimer;


Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP), fovRadius(10),
						    screenWidth(screenWidth), screenHeight(screenHeight),
						    dp(100), lastTurnTime(0)
{
  TCODConsole::initRoot(screenWidth, screenHeight, "foik", false);
  TCODMouse::showCursor(true);
  gui = new Gui();
  intrusionTimer = TCODRandom::getInstance()->getInt(10, 100);
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
  int nbFloors = TCODRandom::getInstance()->getInt(2,4);

  while(nbFloors > 0) {
    Floor *floor = new Floor();
    floor->map = new Map(screenWidth - 40, screenHeight - 7, floor);
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

  gui->message(TCODColor::green, "Welcome to dungeon, master!");
  gameStatus = STARTUP;
}

void Engine::update() {
  if(gameStatus == STARTUP)
    gameStatus = PAUSED;
  
  TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);
  if(lastKey.vk == TCODK_ESCAPE) {
    save();
    load(true);
  }

  if(lastKey.c == '[') {
    currentFloorIndex = std::max(currentFloorIndex - 1, 0);
    currentFloor = floors.get(currentFloorIndex);
  }
  
  if(lastKey.c == ']') {
    currentFloorIndex = std::min(currentFloorIndex + 1, floors.size() - 1);
    currentFloor = floors.get(currentFloorIndex);
  }

  if(lastKey.vk == TCODK_SPACE) {
    if(gameStatus == RUNNING)
      gameStatus = PAUSED;
    else if(gameStatus == PAUSED)
      gameStatus = RUNNING;
  }

  if(lastKey.vk == TCODK_TAB) {
    if(gui->guiStatus == Gui::LOGS)
      gui->guiStatus = Gui::MENU;
    else if(gui->guiStatus == Gui::MENU)
      gui->guiStatus = Gui::LOGS;
  }

  gui->handleActionKey(lastKey.c);

  if(gameStatus == RUNNING) {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    if(ms - lastTurnTime >= turnDelay) {
      lastTurnTime = ms;
      intrusionTimer--;
      if(intrusionTimer <= 0) {
	  intrusionTimer = TCODRandom::getInstance()->getInt(10, 100);
	  spawnIntruder();
	  gui->message(TCODColor::darkRed, "Someone intruded your dungeon!");
      }
      for(auto floor: floors) {
	for(auto actor: floor->actors)
	  actor->update();
      }
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



bool Engine::pickATile(int *x, int *y) {
  while(!TCODConsole::isWindowClosed()) {
    render();
    
    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);

    if (currentFloor->map->canWalk(mouse.cx, mouse.cy)) {
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
  
  gui->save(saver);
  saver.saveToFile("game.sav");
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
      if(!actor->isEnemy && actor->destructible && !actor->destructible->isDead())
	nbMonsters++;
    }
  }
  return nbMonsters;
}

void Engine::spawnIntruder() {
  Actor *adventurer = new Actor(currentFloor, 40, 25, '@', "adventurer", TCODColor::white);
  adventurer->destructible = new AdventurerDestructible(TCODRandom::getInstance()->getInt(5, 10), 2, "human corpse");
  adventurer->attacker = new Attacker(TCODRandom::getInstance()->getInt(1, 5));
  adventurer->ai = new AdventurerAi();
  adventurer->container = new Container(26);
  adventurer->isEnemy = true;

  Floor *firstFloor = floors.get(0);
  
  adventurer->x = firstFloor->map->entryx;
  adventurer->y = firstFloor->map->entryy;
  adventurer->currentFloor = firstFloor;

  firstFloor->actors.push(adventurer);
}
