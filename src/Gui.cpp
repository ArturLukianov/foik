#include "main.hpp"

static const int PANEL_HEIGHT = 7;
static const int BAR_WIDTH = 20;
static int MSG_X;
static int MSG_HEIGHT;
static int MSG_WIDTH;

Gui::Gui() {
  statsCon = new TCODConsole(engine.screenWidth, PANEL_HEIGHT);
  MSG_X = engine.screenWidth - 40 + 2;
  MSG_HEIGHT = engine.screenHeight - 4;
  MSG_WIDTH = 40 - 4;
  guiStatus = LOGS;
  sideMenu = MAIN;
}

Gui::~Gui() {
  delete statsCon;
  clear();
}

void Gui::clear() {
  log.clearAndDelete();
}

void Gui::render() {
  statsCon->setDefaultBackground(TCODColor::black);
  statsCon->clear();

  statsCon->setDefaultForeground(TCODColor::white);
  statsCon->print(1, 1, "Monsters: %d", engine.countMonsters());

  if(engine.gameStatus == Engine::PAUSED) {
    statsCon->setDefaultBackground(TCODColor::darkerYellow);
    statsCon->rect(51, 1, strlen("=PAUSED="), 1, false, TCOD_BKGND_SET);
    statsCon->setDefaultForeground(TCODColor::yellow);
    statsCon->print(51, 1, "=PAUSED=");
  }

  TCODConsole::blit(statsCon, 0, 0, engine.screenWidth, PANEL_HEIGHT,
		    TCODConsole::root, 0, engine.screenHeight - PANEL_HEIGHT);


  if (guiStatus == LOGS) {
    TCODConsole::root->setDefaultBackground(TCODColor::darkerGrey);
    TCODConsole::root->rect(MSG_X + 1, 1, strlen("Logs") + 2, 1, false, TCOD_BKGND_SET);
    TCODConsole::root->setDefaultForeground(TCODColor::white);
    TCODConsole::root->print(MSG_X + 2, 1, "Logs");
    TCODConsole::root->setDefaultBackground(TCODColor::darkestGrey);
    TCODConsole::root->rect(MSG_X + 2 + 6, 1, strlen("Menu") + 2, 1, false, TCOD_BKGND_SET);
    TCODConsole::root->setDefaultForeground(TCODColor::darkGrey);
    TCODConsole::root->print(MSG_X + 2 + 6 + 1, 1, "Menu");
    TCODConsole::root->setDefaultBackground(TCODColor::black);

    renderSideLogs();
  } else if (guiStatus == MENU) {
    TCODConsole::root->setDefaultBackground(TCODColor::darkestGrey);
    TCODConsole::root->rect(MSG_X + 1, 1, strlen("Logs") + 2, 1, false, TCOD_BKGND_SET);
    TCODConsole::root->setDefaultForeground(TCODColor::darkGrey);
    TCODConsole::root->print(MSG_X + 2, 1, "Logs");
    TCODConsole::root->setDefaultBackground(TCODColor::darkerGrey);
    TCODConsole::root->rect(MSG_X + 2 + 6, 1, strlen("Menu") + 2, 1, false, TCOD_BKGND_SET);
    TCODConsole::root->setDefaultForeground(TCODColor::white);
    TCODConsole::root->print(MSG_X + 2 + 6 + 1, 1, "Menu");
    TCODConsole::root->setDefaultBackground(TCODColor::black);

    if(sideMenu == MAIN)
      renderMainMenu();
    else if(sideMenu == BUILD)
      renderBuildMenu();
  }

}

void Gui::renderSideLogs() {
  int y=3;
  float colorCoef = 0.4f;
  for(auto message: log) {
    TCODConsole::root->setDefaultForeground(message->col * colorCoef);
    TCODConsole::root->print(MSG_X, y, message->text);
    y++;
    if(colorCoef < 1.0f)
      colorCoef += 0.3f;
  }
}

void Gui::handleActionKey(char key) {
  if(sideMenu == MAIN) {
    switch(key) {
    case 'b':
      sideMenu = BUILD;
      break;
    }
  } else if(sideMenu == BUILD) {
    switch(key) {
    case 'c':
      int x, y;
      if(engine.pickATile(&x, &y)) {
	Actor *crossbow = new Actor(engine.currentFloor, x, y, '}', "crossbow", TCODColor::violet);
	crossbow->blocks = false;
	crossbow->attacker = new Attacker(6);
	crossbow->destructible = new ConstructionDestructible(2, 0, "broken crossbow");
	crossbow->ai = new RangedConstructionAi(2, 10, 5.0f);
	engine.currentFloor->actors.push(crossbow);
      }
      break;
    }
  }
}

void Gui::renderMainMenu() {
  TCODConsole::root->setDefaultForeground(TCODColor::lightGreen);
  TCODConsole::root->print(MSG_X, 3, "[");
  TCODConsole::root->print(MSG_X + 2, 3, "]");
  TCODConsole::root->setDefaultForeground(TCODColor::white);
  TCODConsole::root->print(MSG_X + 1, 3, "/");
  TCODConsole::root->print(MSG_X + 4, 3, "- move up/down a floor");

  TCODConsole::root->setDefaultForeground(TCODColor::lightGreen);
  TCODConsole::root->print(MSG_X, 4, "SPACE");
  TCODConsole::root->setDefaultForeground(TCODColor::white);
  TCODConsole::root->print(MSG_X + 6, 4, "- pause/unpause");

  TCODConsole::root->setDefaultForeground(TCODColor::lightGreen);
  TCODConsole::root->print(MSG_X, 4, "SPACE");
  TCODConsole::root->setDefaultForeground(TCODColor::white);
  TCODConsole::root->print(MSG_X + 6, 4, "- pause/unpause");
    
  TCODConsole::root->setDefaultForeground(TCODColor::lightGreen);
  TCODConsole::root->print(MSG_X, 5, "b");
  TCODConsole::root->setDefaultForeground(TCODColor::white);
  TCODConsole::root->print(MSG_X + 2, 5, "- build");
}

void Gui::renderBuildMenu() {
  TCODConsole::root->setDefaultForeground(TCODColor::lightViolet);
  TCODConsole::root->print(MSG_X, 3, "Build:");

  
  TCODConsole::root->setDefaultForeground(TCODColor::lightGreen);
  TCODConsole::root->print(MSG_X, 5, "c");
  TCODConsole::root->setDefaultForeground(TCODColor::white);
  TCODConsole::root->print(MSG_X + 2, 5, "- crossbow");
}

void Gui::renderBar(int x, int y, int width, const char *name,
		    float value, float maxValue, const TCODColor &barColor,
		    const TCODColor &backColor) {
  statsCon->setDefaultBackground(backColor);
  statsCon->rect(x, y, width, 1, false, TCOD_BKGND_SET);
  int barWidth = (int)(value / maxValue * width);
  if(barWidth > 0) {
    statsCon->setDefaultBackground(barColor);
    statsCon->rect(x, y, barWidth, 1, false, TCOD_BKGND_SET);
  }

  statsCon->setDefaultForeground(TCODColor::white);
  statsCon->printEx(x+width/2, y, TCOD_BKGND_NONE, TCOD_CENTER, "%s : %g/%g", name, value, maxValue);
}


Gui::Message::Message(const char *text, const TCODColor &col) :
  text(strdup(text)), col(col) {}

Gui::Message::~Message() {
  free(text);
}


void Gui::message(const TCODColor &col, const char *text, ...) {
  va_list ap;
  char buf[128];
  va_start(ap, text);
  vsprintf(buf, text, ap);
  va_end(ap);

  char *lineBegin=buf;
  char *lineEnd;

  do {
    lineEnd = strchr(lineBegin, '\n');
    if(lineEnd) {
      *lineEnd = '\0';
    }
    char buf2[128];
    do {
      if(log.size() == MSG_HEIGHT) {
	Message *toRemove = log.get(0);
	log.remove(toRemove);
	delete toRemove;
      }
      int len = strlen(lineBegin);
      int copylen = len > MSG_WIDTH ? MSG_WIDTH : len;
      strncpy(buf2, lineBegin, copylen);
      buf2[copylen] = '\0';
      lineBegin += copylen;
      
      Message *msg = new Message(buf2, col);
      log.push(msg);
    } while (strlen(lineBegin) != 0);

    lineBegin = lineEnd + 1;
  } while (lineEnd);
}

void Gui::save(Saver &saver) {
  saver.putInt(log.size());
  for(auto message : log) {
    saver.putString(message->text);
    saver.putColor(&message->col);
  }
}

void Gui::load(Saver &saver) {
  clear();
  int nbMessages = saver.getInt();
  while(nbMessages > 0) {
    Message *message = new Message(strdup(saver.getString()), saver.getColor());
    log.push(message);
    nbMessages--;
  }
}


Menu::~Menu() {
  clear();
}

void Menu::clear() {
  items.clearAndDelete();
}

void Menu::addItem(MenuItemCode code, const char *label) {
  MenuItem *item = new MenuItem();
  item->code = code;
  item->label = label;
  items.push(item);
}

const int PAUSE_MENU_WIDTH = 30;
const int PAUSE_MENU_HEIGHT = 15;

Menu::MenuItemCode Menu::pick(DisplayMode mode) {
  int selectedItem=0;
  while(!TCODConsole::isWindowClosed()) {
    TCODConsole::root->clear();

    int currentItem = 0;
    int menux,menuy;
    if(mode == PAUSE) {
      menux = engine.screenWidth / 2 - PAUSE_MENU_WIDTH / 2;
      menuy = engine.screenHeight / 2 - PAUSE_MENU_HEIGHT / 2;
      TCODConsole::root->setDefaultForeground(TCODColor(200, 180, 50));
      TCODConsole::root->printFrame(menux, menuy, PAUSE_MENU_WIDTH, PAUSE_MENU_HEIGHT, true, TCOD_BKGND_ALPHA(70), "menu");
    } else {
      menux = 10;
      menuy = TCODConsole::root->getHeight() / 3;
    }
    menux += 2;
    menuy += 3;
    for(auto item : items) {
      if(currentItem == selectedItem) {
	TCODConsole::root->setDefaultForeground(TCODColor::lightOrange);
      } else {
	TCODConsole::root->setDefaultForeground(TCODColor::lightGrey);
      }
      TCODConsole::root->print(menux, menuy+currentItem * 3, item->label);
      currentItem++;
    }
    TCODConsole::flush();
    TCOD_key_t key;
    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL);
    switch(key.vk) {
    case TCODK_UP:
      selectedItem--;
      if(selectedItem < 0)
	selectedItem = items.size() - 1;
      break;
    case TCODK_DOWN:
      selectedItem = (selectedItem + 1) % items.size();
      break;
    case TCODK_ENTER:
      return items.get(selectedItem)->code;
    default:
      break;
    }
  }
  return NONE;
}
