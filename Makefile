SOURCES=$(wildcard src/*.cpp)
OBJS=$(SOURCES:.cpp=.o)

ifeq ($(shell sh -c 'uname -s'),Linux)
	LIBFLAGS=-L. -ltcod -Wl,-rpath=.
else
	LIBFLAGS=-Llib -ltcod-mingw -static-libgcc -static-libstdc++
endif

foik : $(OBJS)
	g++ $(OBJS) -o foik -Wall $(LIBFLAGS) -g

src/%.o : src/%.cpp
	g++ $< -c -o $@ -Iinclude -Wall -g

clean :
	rm -f $(OBJS)
