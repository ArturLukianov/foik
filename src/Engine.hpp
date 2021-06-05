class Engine {
public:
  int screenWidth;
  int screenHeight;
  Gui *gui;
  TCOD_key_t lastKey;
  TCOD_mouse_t mouse;

  int dp;
  
  enum GameStatus {
    STARTUP,
    RUNNING,
    PAUSED,
    DEFEAT
  } gameStatus;
  
  int fovRadius;

  TCODList<Floor *> floors;
  Floor *currentFloor;
  int currentFloorIndex;

  Engine(int screenWidth, int screenHeight);
  ~Engine();
  void update();
  void render();
  //Actor *getClosestMonster(int x, int y, float range) const;

  bool pickATile(int *x, int *y);
  void init();
  void load(bool pause = false);
  void save();
  void term();
  int getFloorIndex(Floor *needle) const;

  int countMonsters() const;

  void spawnIntruder();

  long int lastTurnTime;
private:
};

extern Engine engine;
