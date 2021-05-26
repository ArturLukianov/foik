class Portal : public Persistent {
public:
  int destinationFloorIndex;
  int targetx, targety;

  Portal(int destinationFloorIndex, int targetx, int targety);
  void warp(Actor *owner, Actor *warper);

  void load(Saver &saver);
  void save(Saver &saver);
};
