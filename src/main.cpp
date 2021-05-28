#include "main.hpp"

Engine engine(80, 60);

int main() {
  engine.load();
  while ( !TCODConsole::isWindowClosed() ) {
    engine.update();
    engine.render();
    TCODConsole::flush();
    //    usleep(100000);
  }
  engine.save();
  return 0;
}
