#include "main.hpp"


static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;

static const int MAX_ROOM_MONSTERS = 3;

static const int MAX_ROOM_ITEMS = 2;


class BspListener : public ITCODBspCallback{
private:
  Map &map;
  int roomNum;
  int lastx, lasty;

public:
  BspListener(Map &map) : map(map), roomNum(0) {}
  bool visitNode(TCODBsp *node, void *userData) {
    if(node->isLeaf()) {
      int x, y, w, h;

      bool withActors = (bool) userData;
      w = map.rng->getInt(ROOM_MIN_SIZE, node->w - 2);
      h = map.rng->getInt(ROOM_MIN_SIZE, node->h - 2);
      x = map.rng->getInt(node->x + 1, node->x + node->w - w - 1);
      y = map.rng->getInt(node->y + 1, node->y + node->h - h - 1);
      map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1, withActors);


      if(roomNum != 0) {
	map.dig(lastx, lasty, x+w/2, lasty);
	map.dig(x+w/2, lasty, x+w/2, y+h/2);
      }
      
      roomNum++;
      lastx = x + w / 2;
      lasty = y + h / 2;
    }
    return true;
  }
};


Map::Map(int width, int height, Floor *floor) : width(width), height(height), floor(floor) {
  seed = TCODRandom::getInstance()->getInt(0, 0x7FFFFFFF);
}

void Map::init(bool withActors) {
  rng = new TCODRandom(seed, TCOD_RNG_CMWC);
  tiles = new Tile[width*height];
  map = new TCODMap(width, height);
  TCODBsp bsp(0, 0, width, height);
  bsp.splitRecursive(rng, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
  BspListener listener(*this);
  bsp.traverseInvertedLevelOrder(&listener, (void *)withActors);
}

Map::~Map() {
  delete [] tiles;
  delete map;
}

bool Map::isWall(int x, int y) const {
  return !map->isWalkable(x, y);
}

bool Map::isExplored(int x, int y) const {
  return tiles[x + y * width].explored;
}

bool Map::isInFov(int x, int y) const {
  if(x < 0 || y < 0 || x >= width || y >= height) return false;
  if(map->isInFov(x, y)) {
    tiles[x + y * width].explored = true;
    return true;
  }
  return false;
}

void Map::computeFov() {
  map->computeFov(engine.player->x, engine.player->y, engine.fovRadius);
}

void Map::dig(int x1, int y1, int x2, int y2) {
  if(x2 < x1) {
    int temp = x2;
    x2 = x1;
    x1 = temp;
  }

  if(y2 < y1) {
    int temp = y2;
    y2 = y1;
    y1 = temp;
  }

  for (int tilex = x1; tilex <= x2; tilex++) {
    for (int tiley = y1; tiley <= y2; tiley++) {
      map->setProperties(tilex, tiley, true, true);
    }
  }
}

void Map::createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors) {
  dig(x1, y1, x2, y2);
  if(!withActors) {
    return;
  }

  if(first) {
    entryx = (x1 + x2) / 2;
    entryy = (y1 + y2) / 2;
  } else {
    TCODRandom *rng = TCODRandom::getInstance();
    int nbMonsters = rng->getInt(0, MAX_ROOM_MONSTERS);
    while(nbMonsters > 0) {
      int x = rng->getInt(x1, x2);
      int y = rng->getInt(y1, y2);
      if(canWalk(x, y))
	addMonster(x, y);
      nbMonsters--;
    }

    int nbItems = rng->getInt(0, MAX_ROOM_ITEMS);
    while(nbItems > 0) {
      int x = rng->getInt(x1, x2);
      int y = rng->getInt(y1, y2);
      if(canWalk(x, y))
	addItem(x, y);
      nbItems--;
    }
  }
}

void Map::render() const {
  static const TCODColor darkWall(0, 0, 100);
  static const TCODColor darkGround(50, 50, 100);
  static const TCODColor lightWall(130, 110, 50);
  static const TCODColor lightGround(200, 180, 50);

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if(isInFov(x, y)) {
	TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? lightWall : lightGround);
      } else if (isExplored(x, y)) {
	TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? darkWall : darkGround);
      }
    }
  }
}

bool Map::canWalk(int x, int y) const {
  if(isWall(x, y))
    return false;

  for(auto actor : floor->actors) {
    if(actor->x == x && actor->y == y && actor->blocks) {
      return false;
    }
  }

  return true;
}


void Map::addMonster(int x, int y) {
  TCODRandom *rng = TCODRandom::getInstance();
  if(rng->getInt(0, 100) < 80) {
    Actor *orc = new Actor(x, y, 'o', "orc", TCODColor::desaturatedGreen);
    orc->destructible = new MonsterDestructible(10, 0, "dead orc", 1);
    orc->attacker = new Attacker(3);
    orc->ai = new MonsterAi();
    floor->actors.push(orc);
  } else {
    Actor *troll = new Actor(x, y, 't', "troll", TCODColor::darkerGreen);
    troll->destructible = new MonsterDestructible(16, 1, "dead troll", 5);
    troll->attacker = new Attacker(4);
    troll->ai = new MonsterAi();
    floor->actors.push(troll);
  }
}

void Map::addItem(int x, int y) {
  TCODRandom *rng = TCODRandom::getInstance();
  int dice = rng->getInt(0, 100);
  if(dice < 70) {
    Actor *healthPotion = new Actor(x, y, '!', "health potion", TCODColor::violet);
    healthPotion->blocks = false;
    healthPotion->pickable = new Healer(4);
    floor->actors.push(healthPotion);
  } else if (dice < 70 + 10) {
    Actor *lightningBolt = new Actor(x, y, '#', "lightning bolt", TCODColor::yellow);
    lightningBolt->blocks = false;
    lightningBolt->pickable = new LightningBolt(5, 20);
    floor->actors.push(lightningBolt);
  } else if (dice < 70 + 10 + 10) {
    Actor *fireball = new Actor(x, y, '#', "fireball", TCODColor::orange);
    fireball->blocks = false;
    fireball->pickable = new Fireball(5, 20);
    floor->actors.push(fireball);
  } else {
    Actor *confuser = new Actor(x, y, '#', "confuser", TCODColor::blue);
    confuser->blocks = false;
    confuser->pickable = new Confuser(10, 8);
    floor->actors.push(confuser);
  }
}

void Map::save(Saver &saver) {
  saver.putInt(seed);
  for(int i = 0; i < width*height; i++) {
    saver.putInt(tiles[i].explored);
  }
}

void Map::load(Saver &saver) {
  seed = saver.getInt();
  init(false);
  for(int i = 0; i < width*height; i++) {
    tiles[i].explored = saver.getInt();
  }
}

void Map::addPortal(Portal *portal) {
  int x = entryx + TCODRandom::getInstance()->getInt(-2, 2);
  int y = entryy + TCODRandom::getInstance()->getInt(-2, 2);
  Actor *portalActor = new Actor(x, y, '0', "portal", TCODColor::black);
  portalActor->fovOnly = false;
  portalActor->portal = portal;
  portalActor->blocks = false;

  floor->actors.push(portalActor);
}