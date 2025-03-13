# Compiler
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -Dmain=SDL_main -IC:/msys64/ucrt64/include/SDL2

# Linker flags
LDFLAGS := -LC:/msys64/ucrt64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -mwindows

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Tên file đầu ra
TARGET := $(BIN_DIR)/game.exe

# Lấy tất cả các file .cpp trong thư mục src
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Build chương trình
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Biên dịch từng file .cpp thành .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Tạo thư mục nếu chưa có
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Chạy game
run: $(TARGET)
	./$(TARGET)

# Dọn dẹp file object và executable
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
