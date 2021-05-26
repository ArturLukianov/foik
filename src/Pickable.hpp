class Pickable : public Persistent{
public:
  virtual ~Pickable() {};
  bool pick(Actor *owner, Actor *wearer);
  void drop(Actor *owner, Actor *wearer);
  virtual bool use(Actor *owner, Actor *wearer);
  
  static Pickable * create(Saver &saver);
protected:
  enum PickableType {
    HEALER, LIGHTNING_BOLT, FIREBALL, CONFUSER
  };
};


class Healer : public Pickable {
public:
  float amount;

  Healer(float amount);
  bool use(Actor *owner, Actor *wearer);

  void save(Saver &saver);
  void load(Saver &saver);
};


class LightningBolt : public Pickable {
public:
  float range, damage;
  LightningBolt(float range, float damage);
  bool use(Actor *owner, Actor *wearer);

  void save(Saver &saver);
  void load(Saver &saver);
};


class Fireball : public LightningBolt {
public:
  Fireball(float range, float damage);
  bool use(Actor *owner, Actor *wearer);

  void save(Saver &saver);
};


class Confuser : public Pickable {
public:
  int nbTurns;
  float range;
  Confuser(int nbTurns, float range);
  bool use(Actor *owner, Actor *wearer);

  void save(Saver &saver);
  void load(Saver &saver);
};
