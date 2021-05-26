class Attacker : public Persistent{
public:
  float power;

  Attacker(float power);
  void attack(Actor *owner, Actor *target);
  void load(Saver &saver);
  void save(Saver &saver);
};
