class Floor : public Persistent {
public:
  Map *map;
  TCODList<Actor *>actors;

  Floor();
  ~Floor();

  void save(Saver &saver);
  void load(Saver &saver);
};
