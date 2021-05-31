class Actor : public Persistent {
public:
  Floor *currentFloor;
  int x, y;
  int ch;
  TCODColor col;
  const char * name;
  bool blocks;
  bool fovOnly;
  Attacker *attacker;
  Destructible *destructible;
  Ai *ai;
  Pickable *pickable;
  Container *container;
  Portal *portal;

  bool isEnemy;

  Actor(Floor *currentFloor, int x, int y, int ch, const char * name, const TCODColor &col);
  ~Actor();
  void render() const;
  void update();
  bool moveOrAttack(int x, int y);
  float getDistance(int cx, int xy) const;

  void save(Saver &saver);
  void load(Saver &saver);
};
