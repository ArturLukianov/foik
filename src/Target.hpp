class Target {
public:
  virtual int getX() = 0;
  virtual int getY() = 0;
  virtual bool isDead() = 0;
  virtual Floor *getFloor() = 0;
};


class ActorTarget : public Target {
public:
  Actor *actor;
  ActorTarget(Actor *actor);
  int getX();
  int getY();
  Floor *getFloor();
  bool isDead();
};
