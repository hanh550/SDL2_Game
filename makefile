CC := g++
CFLAGS := -Wall -Wextra -std=c++11 `sdl2-config --cflags`
LDFLAGS := `sdl2-config --libs` -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(SOURCES:src/%.cpp=src/%.o)
TARGET := main.exe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

src/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: all
	./$(TARGET)
