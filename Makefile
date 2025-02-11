# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Compiler and flags
CXX = g++
CXXFLAGS_COMMON = -std=c++17
ifeq ($(shell uname), Darwin)
	CXXFLAGS = -std=c++17 $(CXXFLAGS_COMMON)  # macOS-specific flags
else
	CXXFLAGS = -std=c++17 $(CXXFLAGS_COMMON)  # Default flags
endif

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.d)

TARGET = $(BIN_DIR)/lexer

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(TARGET): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | directories
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEP_FILES)

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild directories