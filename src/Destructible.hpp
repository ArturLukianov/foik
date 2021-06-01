class Destructible : public Persistent {
public:
  int xp;
  float maxHp;
  float hp;
  float defense;
  const char *corpseName;

  Destructible(float maxHp, float defense, const char *corpseName, int xp);
  virtual ~Destructible();
  inline bool isDead() { return hp <= 0; }
  float takeDamage(Actor *owner, Actor *dealer, float damage);
  float countDamage(Actor *owner, float damage);
  virtual void die(Actor *owner, Actor *killer);
  float heal(float amount);

  void load(Saver &saver);
  void save(Saver &saver);
  static Destructible *create(Saver &saver);
protected:
  enum DestructibleType {
    MONSTER, PLAYER
  };
};


class PlayerDestructible : public Destructible {
public:
  PlayerDestructible(float maxHp, float defense, const char *corpseName);
  void die(Actor *owner, Actor *killer);
  void save(Saver &saver);
};

class MonsterDestructible : public Destructible {
public:
  MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp);
  void die(Actor *owner, Actor *killer);
  void save(Saver &saver);
};
