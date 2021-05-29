struct Tile {
  bool explored;
  Tile() : explored(false) {}
};

class Map : public Persistent{
public:
  int width, height;
  Floor *floor;

  int entryx, entryy;

  Map(int width, int height, Floor *floor);
  ~Map();
  bool isWall(int x, int y) const;
  void render() const;

  bool isInFov(int x, int y) const;
  bool isExplored(int x, int y) const;
  void computeFov();
  bool canWalk(int x, int y) const;
  void init(bool withActors);
  void load(Saver &saver);
  void save(Saver &saver);

  std::pair<int, int> addUpStairs(Actor *actor);
  std::pair<int, int> addDownStairs(Actor *actor);
protected:
  long seed;
  TCODRandom *rng;
  Tile *tiles;
  friend class BspListener;
  TCODMap *map;

  void dig(int x1, int y1, int x2, int y2);
  void createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors);
  void addMonster(int x, int y);
  void addItem(int x, int y);
};
