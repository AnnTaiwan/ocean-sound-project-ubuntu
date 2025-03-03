CXX := g++
CXXFLAGS := -std=c++17 -O2

# Use pkg-config to get necessary GStreamer flags
GSTREAMER_FLAGS := $(shell pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0)

# Include directory
INCLUDE_DIR := ./

# Target executable
TARGET := app
OBJ := app.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GSTREAMER_FLAGS)

$(OBJ): app.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(GSTREAMER_FLAGS) -I$(INCLUDE_DIR)

clean:
	rm -f $(OBJ) $(TARGET)

