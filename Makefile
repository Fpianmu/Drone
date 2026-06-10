# 无人机编队灯光秀模拟系统 - Makefile
# 编译环境: MinGW-w64 / GCC

CXX      = g++
CXXFLAGS = -std=c++11 -Wall
TARGET   = drone_show.exe
SRC_DIR  = src
INC_DIR  = include

SOURCES  = main.cpp \
           $(SRC_DIR)/drone.cpp \
           $(SRC_DIR)/light.cpp \
           $(SRC_DIR)/formation.cpp \
           $(SRC_DIR)/trajectory.cpp \
           $(SRC_DIR)/safety.cpp \
           $(SRC_DIR)/graphics.cpp \
           $(SRC_DIR)/ui.cpp \
           $(SRC_DIR)/file_io.cpp \
           $(SRC_DIR)/controller.cpp

INCLUDES = -I $(INC_DIR)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDES)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
