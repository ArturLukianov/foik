class Ai : public Persistent{
public:
  virtual ~Ai() {};
  virtual void update(Actor *owner)=0;
  static Ai * create(Saver &saver);
protected:
  enum AiType {
    PLAYER, MONSTER, CONFUSED_MONSTER
  };
};


class PlayerAi : public Ai {
public:
  int xpLevel;
  PlayerAi();
  int getNextLevelXp();
  void handleActionKey(Actor *owner, int ascii);
  Actor *chooseFromInventory(Actor *owner);
  void update(Actor *owner);
  void save(Saver &saver);
  void load(Saver &saver);

protected:
  bool moveOrAttack(Actor *owner, int targetx, int targety);
};


class MonsterAi : public Ai {
public:
  MonsterAi();
  void update(Actor *owner);
  void load(Saver &saver);
  void save(Saver &saver);

protected:
  int moveCount;
  bool moveOrAttack(Actor *owner, int targetx, int targety);
};


class ConfusedMonsterAi : public Ai {
public:
  ConfusedMonsterAi(int nbTurns, Ai *oldAi);
  void update(Actor *owner);
  void load(Saver &saver);
  void save(Saver &saver);
  
protected:
  int nbTurns;
  Ai *oldAi;
};
