class Saver {
public:
  Saver();
  ~Saver();
  
  static bool fileExists(const char *path);
  static void deleteFile(const char *path);
  void saveToFile(const char *path);
  void loadFromFile(const char *path);

  void putInt(int val);
  void putChar(char val);
  void putColor(const TCODColor *val);
  void putString(const char *val);
  void putFloat(float val);

  int getInt();
  char getChar();
  TCODColor getColor();
  const char * getString();
  float getFloat();
  
private:
  std::stringstream buf;
};

class Persistent {
public:
  virtual void load(Saver &saver) = 0;
  virtual void save(Saver &saver) = 0;
};
