#include "main.hpp"

static const int PANEL_HEIGHT = 7;
static const int BAR_WIDTH = 20;
static const int MSG_X = BAR_WIDTH + 2;
static const int MSG_HEIGHT = PANEL_HEIGHT - 1;

Gui::Gui() {
  con = new TCODConsole(engine.screenWidth, PANEL_HEIGHT);
}

Gui::~Gui() {
  delete con;
  clear();
}

void Gui::clear() {
  log.clearAndDelete();
}

void Gui::render() {
  con->setDefaultBackground(TCODColor::black);
  con->clear();

  con->setDefaultForeground(TCODColor::white);
  con->print(1, 1, "Monsters: %d", engine.countMonsters());

  int y=1;
  float colorCoef = 0.4f;
  for(auto message: log) {
    con->setDefaultForeground(message->col * colorCoef);
    con->print(MSG_X, y, message->text);
    y++;
    if(colorCoef < 1.0f)
      colorCoef += 0.3f;
  }

  renderMouseLook();

  TCODConsole::blit(con, 0, 0, engine.screenWidth, PANEL_HEIGHT,
		    TCODConsole::root, 0, engine.screenHeight - PANEL_HEIGHT);

}

void Gui::renderBar(int x, int y, int width, const char *name,
		    float value, float maxValue, const TCODColor &barColor,
		    const TCODColor &backColor) {
  con->setDefaultBackground(backColor);
  con->rect(x, y, width, 1, false, TCOD_BKGND_SET);
  int barWidth = (int)(value / maxValue * width);
  if(barWidth > 0) {
    con->setDefaultBackground(barColor);
    con->rect(x, y, barWidth, 1, false, TCOD_BKGND_SET);
  }

  con->setDefaultForeground(TCODColor::white);
  con->printEx(x+width/2, y, TCOD_BKGND_NONE, TCOD_CENTER, "%s : %g/%g", name, value, maxValue);
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
    if(log.size() == MSG_HEIGHT) {
      Message *toRemove = log.get(0);
      log.remove(toRemove);
      delete toRemove;
    }
    lineEnd = strchr(lineBegin, '\n');
    if(lineEnd) {
      *lineEnd = '\0';
    }
    Message *msg = new Message(lineBegin, col);
    log.push(msg);
    lineBegin = lineEnd + 1;
  } while (lineEnd);
}

void Gui::renderMouseLook() {
  if(!engine.currentFloor->map->isInFov(engine.mouse.cx, engine.mouse.cy))
    return;
  char buf[128]="";
  bool first = true;
  for(auto actor : engine.currentFloor->actors) {
    if(actor->x == engine.mouse.cx && actor->y == engine.mouse.cy) {
      if(!first)
	strcat(buf, ", ");
      else
	first = false;
      strcat(buf, actor->name);
    }
  }
  con->setDefaultForeground(TCODColor::lightGrey);
  con->print(1,0,buf);
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
