class Ai : public Persistent{
public:
  virtual ~Ai() {};
  virtual void update(Actor *owner)=0;
  static Ai * create(Saver &saver);
protected:
  enum AiType {
	       ADVENTURER, MONSTER, CONFUSED_MONSTER, RANGED_CONSTRUCTION
  };
};


class AdventurerAi : public Ai {
public:
  enum AdventurerState {
    EXPLORE,
    PICK_ITEM,
    ATTACK_MONSTER,
    NEXT_FLOOR
  };

  AdventurerState state;

  Target *target;
  
  int xpLevel;
  AdventurerAi();
  int getNextLevelXp();
  //void handleActionKey(Actor *owner, int ascii);
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
  Actor *getMonsterInFov(Actor *owner);
  Actor *getItemInFov(Actor *owner);
 

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
  Target *target;
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


class RangedConstructionAi : public Ai {
public:
  RangedConstructionAi(int maxBolts, int boltRechargeTime, float range);
  void update(Actor *owner);
  void load(Saver &saver);
  void save(Saver &saver);

protected:
  Target *target;
  int maxBolts;
  int bolts;
  int boltRechargeTime;
  int rechargeTimer;
  float range;
  bool moveOrAttack(Actor *owner, int targetx, int targety);
};
