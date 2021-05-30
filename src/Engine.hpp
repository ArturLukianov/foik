class Engine {
public:
  int screenWidth;
  int screenHeight;
  Gui *gui;
  TCOD_key_t lastKey;
  TCOD_mouse_t mouse;
  
  enum GameStatus {
    STARTUP,
    IDLE,
    NEW_TURN,
    VICTORY,
    DEFEAT
  } gameStatus;
  
  int fovRadius;
  Actor *player;

  TCODList<Floor *> floors;
  Floor *currentFloor;
  int currentFloorIndex;

  Engine(int screenWidth, int screenHeight);
  ~Engine();
  void update();
  void render();
  Actor *getClosestMonster(int x, int y, float range) const;

  bool pickATile(int *x, int *y, float maxRange = 0.0f);
  Actor *getActor(int x, int y) const;
  void init();
  void load(bool pause = false);
  void save();
  void term();
  int getFloorIndex(Floor *needle) const;
  Actor *getPortal(int x, int y) const;
private:
};

extern Engine engine;
