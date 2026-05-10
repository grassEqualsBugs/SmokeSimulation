CC      = clang++
CFLAGS  = -std=c++17 -O3 -I include/ -MMD -MP
LDFLAGS = -L lib/ -framework CoreVideo -framework IOKit \
          -framework Cocoa -framework GLUT -framework OpenGL \
          lib/libraylib.a

SRC_DIR = .
OBJ_DIR = obj
TARGET  = smoke_cpu

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS    = $(OBJECTS:.o=.d)

default: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: default clean
