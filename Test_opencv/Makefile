CXX := g++
CXXFLAGS := -std=c++17 -O2 

# Use pkg-config to get necessary GStreamer and Cairo flags
CAIRO_FLAGS := $(shell pkg-config --cflags --libs cairo)
OPENCV_FLAGS := $(shell pkg-config --cflags --libs opencv4)
# Include directory
INCLUDE_DIR := ./

# Target file
TARGET := plot_pic
OBJ := plot_pic.o
SOURCE := plot_pic.cpp


TARGET_2 := see_opencv_rst
OBJ_2 := see_opencv_rst.o
SOURCE_2 := see_opencv_rst.cpp

all: $(TARGET) $(TARGET_2)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(CAIRO_FLAGS)

$(OBJ): $(SOURCE)
	$(CXX) $(CXXFLAGS) -c $< -o $@  $(CAIRO_FLAGS) 

$(TARGET_2): $(OBJ_2)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(OPENCV_FLAGS)

$(OBJ_2): $(SOURCE_2)
	$(CXX) $(CXXFLAGS) -c $< -o $@  $(OPENCV_FLAGS) 

clean:
	rm -f $(OBJ) $(TARGET)  $(OBJ_2) $(TARGET_2)


