#include "main.hpp"


Portal::Portal(int destinationFloorIndex, int targetx, int targety) :
  destinationFloorIndex(destinationFloorIndex),
  targetx(targetx), targety(targety) {

}

void Portal::warp(Actor *owner, Actor *warper) {
  if(warper == engine.player) {
    Floor *floor = engine.floors.get(destinationFloorIndex);
    engine.gui->message(TCODColor::orange, "You moved to another location");
    warper->currentFloor->actors.remove(warper);
    warper->currentFloor = floor;
    warper->currentFloor->actors.push(warper);
    
    warper->x = targetx;
    warper->y = targety;
  }
}

void Portal::save(Saver &saver) {
  saver.putInt(destinationFloorIndex);
  saver.putInt(targetx);
  saver.putInt(targety);
}

void Portal::load(Saver &saver) {
  destinationFloorIndex = saver.getInt();
  targetx = saver.getInt();
  targety = saver.getInt();
}
