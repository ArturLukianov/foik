#include "main.hpp"

Destructible::Destructible(float maxHp, float defense, const char *corpseName, int xp) :
  maxHp(maxHp), hp(maxHp), defense(defense), corpseName(strdup(corpseName)), xp(xp) {}

Destructible::~Destructible() {
  free((char *)corpseName);
}

float Destructible::takeDamage(Actor *owner, Actor *dealer, float damage) {
  damage -= defense;
  if (damage > 0) {
    hp -= damage;
    if (hp <= 0) {
      die(owner, dealer);
    }
  } else {
    damage = 0;
  }
  return damage;
}

float Destructible::countDamage(Actor *owner, float damage) {
  damage -= defense;
  if(damage < 0)
    damage = 0;
  return damage;
}

void Destructible::die(Actor *owner, Actor *killer) {
  owner->ch = '%';
  owner->col = TCODColor::red;
  owner->name = corpseName;
  owner->blocks = false;

  owner->currentFloor->sendToBack(owner);
}

float Destructible::heal(float amount) {
  hp += amount;
  if(hp > maxHp) {
    amount -= hp - maxHp;
    hp=maxHp;
  }
  return amount;
}

void Destructible::save(Saver &saver) {
  saver.putFloat(maxHp);
  saver.putFloat(hp);
  saver.putFloat(defense);
  saver.putString(corpseName);
  saver.putInt(xp);
}

void Destructible::load(Saver &saver) {
  maxHp = saver.getFloat();
  hp = saver.getFloat();
  defense = saver.getFloat();
  corpseName = strdup(saver.getString());
  xp = saver.getInt();
}

Destructible *Destructible::create(Saver &saver) {
  DestructibleType type = (DestructibleType)saver.getInt();
  Destructible *destructible = NULL;
  switch(type) {
  case MONSTER: destructible = new MonsterDestructible(0.0f, 0.0f, "", 0); break;
  case PLAYER: destructible = new PlayerDestructible(0.0f, 0.0f, ""); break;
  }
  destructible->load(saver);
  return destructible;
}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp) :
  Destructible(maxHp, defense, corpseName, xp) {}

void MonsterDestructible::die(Actor *owner, Actor *killer) {
  engine.gui->message(TCODColor::red, "%s is dead. %s gain %d xp", owner->name, killer->name, xp);
  killer->destructible->xp += xp;
  Destructible::die(owner, killer);
}


void MonsterDestructible::save(Saver &saver) {
  saver.putInt(MONSTER);
  Destructible::save(saver);
}


ConstructionDestructible::ConstructionDestructible(float maxHp, float defense, const char *corpseName) :
  Destructible(maxHp, defense, corpseName, 0) {}

void ConstructionDestructible::die(Actor *owner, Actor *killer) {
  engine.gui->message(TCODColor::red, "%s is broken by %s.", owner->name, killer->name);
  Destructible::die(owner, killer);
}


void ConstructionDestructible::save(Saver &saver) {
  saver.putInt(CONSTRUCTION);
  Destructible::save(saver);
}


PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char *corpseName) :
  Destructible(maxHp, defense, corpseName, 0) {}

void PlayerDestructible::save(Saver &saver) {
  saver.putInt(PLAYER);
  Destructible::save(saver);
}

void PlayerDestructible::die(Actor *owner, Actor *killer) { 
  engine.gui->message(TCODColor::red, "%s is dead. %s gain %d xp", owner->name, killer->name, xp);
  killer->destructible->xp += xp;
  Destructible::die(owner, killer);
}
