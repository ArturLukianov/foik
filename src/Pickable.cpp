#include "main.hpp"


bool Pickable::pick(Actor *owner, Actor *wearer) {
  if(wearer->container && wearer->container->add(owner)) {
    owner->currentFloor->actors.remove(owner);
    return true;
  }
  return false;
}

void Pickable::drop(Actor *owner, Actor *wearer) {
  if(wearer->container) {
    wearer->container->remove(owner);
    owner->currentFloor->actors.push(owner);
    owner->x = wearer->x;
    owner->y = wearer->y;
    engine.gui->message(TCODColor::lightGrey, "%s drops %s", wearer->name, owner->name);
  }
}

bool Pickable::use(Actor *owner, Actor *wearer) {
  if(wearer->container) {
    wearer->container->remove(owner);
    delete owner;
    return true;
  }
  return false;
}

Pickable * Pickable::create(Saver &saver) {
  PickableType type = (PickableType)saver.getInt();
  Pickable *pickable = NULL;
  switch(type) {
  case HEALER: pickable = new Healer(0.0f); break;
  case LIGHTNING_BOLT: pickable = new LightningBolt(0.0f, 0.0f); break;
  case FIREBALL: pickable = new Fireball(0.0f, 0.0f); break;
  case CONFUSER: pickable = new Confuser(0, 0.0f); break;
  }
  pickable->load(saver);
  return pickable;
}


Healer::Healer(float amount) : amount(amount) {
}

bool Healer::use(Actor *owner, Actor *wearer) {
  if(wearer->destructible) {
    float amountHealed = wearer->destructible->heal(amount);
    if(amountHealed > 0) {
      engine.gui->message(TCODColor::lightBlue, "You quaffed %s and healed %g hit points", owner->name, amountHealed);
      return Pickable::use(owner, wearer);
    }
  }
  return false;
}

void Healer::load(Saver &saver) {
  amount = saver.getInt();
}

void Healer::save(Saver &saver) {
  saver.putInt(HEALER);
  saver.putInt(amount);
}


LightningBolt::LightningBolt(float range, float damage) : range(range), damage(damage) {}

bool LightningBolt::use(Actor *owner, Actor *wearer) {
  Actor *closestMonster = wearer->currentFloor->getClosestMonster(wearer->x, wearer->y, range);
  if(!closestMonster) {
    engine.gui->message(TCODColor::lightGrey, "No enenmy is close enough to strike");
    return false;
  }

  engine.gui->message(TCODColor::lightBlue,
		      "A lightning bolt strikes the %s with a loud thunder!\n"
		      "The damage is %g hit points",
		      closestMonster->name, closestMonster->destructible->countDamage(closestMonster, damage));

  closestMonster->destructible->takeDamage(closestMonster, wearer, damage);
  return Pickable::use(owner, wearer);
}

void LightningBolt::save(Saver &saver) {
  saver.putInt(LIGHTNING_BOLT);
  saver.putFloat(range);
  saver.putFloat(damage);
}

void LightningBolt::load(Saver &saver) {
  range = saver.getFloat();
  damage = saver.getFloat();
}


Fireball::Fireball(float range, float damage) : LightningBolt(range, damage) {}

bool Fireball::use(Actor *owner, Actor *wearer) {
  /*  engine.gui->message(TCODColor::cyan, "Left-click a target tile for the fireball,\nor right-click to cancel.");
  int x,y;
  if(!engine.pickATile(&x, &y, range)) {
    return false;
  }

  for(auto actor : engine.currentFloor->actors) {
    if(actor->destructible && !actor->destructible->isDead() && actor->getDistance(x, y) <= range) {
      engine.gui->message(TCODColor::orange, "The %s gets burned for %g hp", actor->name, actor->destructible->countDamage(actor, damage));
      actor->destructible->takeDamage(actor, wearer, damage);
    }
    }*/

  return Pickable::use(owner, wearer);
}

void Fireball::save(Saver &saver) {
  saver.putInt(FIREBALL);
  saver.putFloat(range);
  saver.putFloat(damage);
}


Confuser::Confuser(int nbTurns, float range) : nbTurns(nbTurns), range(range) {}

bool Confuser::use(Actor *owner, Actor *wearer) {
  /*  engine.gui->message(TCODColor::cyan, "Left-click a target tile for the confuser,\nor right-click to cancel.");
  int x,y;
  if(!engine.pickATile(&x, &y, range)) {
    return false;
  }

  Actor *actor = wearer->currentFloor->getActor(x, y);

  if(!actor) {
    engine.gui->message(TCODColor::lightGrey, "There is nothing to confuse", actor->name);
    return false;
  }

  Ai *confusedAi = new ConfusedMonsterAi(nbTurns, actor->ai);

  engine.gui->message(TCODColor::orange, "The %s is confused!", actor->name);
  actor->ai = confusedAi;*/

  return Pickable::use(owner, wearer);
}

void Confuser::save(Saver &saver) {
  saver.putInt(CONFUSER);
  saver.putInt(nbTurns);
  saver.putFloat(range);
}

void Confuser::load(Saver &saver) {
  nbTurns = saver.getInt();
  range = saver.getFloat();
}
