#include "main.hpp"

Engine engine(100, 70);

int main() {
  engine.load();
  while ( !TCODConsole::isWindowClosed() ) {
    engine.update();
    engine.render();
    TCODConsole::flush();
  }
  engine.save();
  TCODConsole::setFullscreen(false);
  return 0;
}
