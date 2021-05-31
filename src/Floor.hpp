class Floor : public Persistent {
public:
  Map *map;
  TCODList<Actor *>actors;

  Floor();
  ~Floor();

  void save(Saver &saver);
  void load(Saver &saver);
  void sendToBack(Actor *actor);
  Actor* getActor(int x, int y) const;
  Actor* getClosestMonster(int x, int y, float range) const;
  Actor* getPortal(int x, int y) const;

};
