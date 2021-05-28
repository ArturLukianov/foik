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
  enum PlayerState {
    EXPLORE,
    PICK_ITEM,
    ATTACK_MONSTER,
    NEXT_FLOOR
  };

  PlayerState state;
  
  int xpLevel;
  PlayerAi();
  int getNextLevelXp();
  void handleActionKey(Actor *owner, int ascii);
  Actor *chooseFromInventory(Actor *owner);
  void update(Actor *owner);
  void save(Saver &saver);
  void load(Saver &saver);

  bool getExploreMove(Actor *owner, int *dx, int *dy);
  bool getPickMove(Actor *owner, int *dx, int *dy);
  bool getAttackMove(Actor *owner, int *dx, int *dy);
  bool getPortalMove(Actor *owner, int *dx, int *dy);

  void pickItemFromTile(Actor *owner);

  bool isItemInFov(Actor *owner);
  bool isMonsterInFov(Actor *owner);

  bool isMonsterOnTile(Actor *owner, int x, int y);

  bool getNextFloorPortal(Actor *owner, int *x, int *y);
  
  void getItemInFovPos(Actor *owner, int *x, int *y);
  void getMonsterInFovPos(Actor *owner, int *x, int *y);
 

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
