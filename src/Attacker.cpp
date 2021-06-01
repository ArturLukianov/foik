#include "main.hpp"


Attacker::Attacker(float power) : power(power) {}

void Attacker::attack(Actor *owner, Actor *target) {
  if(target->destructible && !target->destructible->isDead()) {
    if(power - target->destructible->defense > 0) {
      engine.gui->message(TCODColor::lightGrey, "%s attacks %s for %g hit points", owner->name, target->name, target->destructible->countDamage(target, power));
      target->destructible->takeDamage(target, owner, power);
    } else {
      engine.gui->message(TCODColor::lightGrey, "%s attacks %s but it has no effect!", owner->name, target->name);
    }
  } else {
    engine.gui->message(TCODColor::lightGrey, "%s attacks %s in vain", owner->name, target->name);
  }
}

void Attacker::save(Saver &saver) {
  saver.putFloat(power);
}

void Attacker::load(Saver &saver) {
  power = saver.getFloat();
}
