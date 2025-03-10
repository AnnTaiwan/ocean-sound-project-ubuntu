CXX := g++
CXXFLAGS := -std=c++17 -O2 -pthread

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

TARGET_3 := cv_pthread
OBJ_3 := cv_pthread.o
SOURCE_3 := cv_pthread.cpp

TARGET_4 := cv_thread_mutex
OBJ_4 := cv_thread_mutex.o
SOURCE_4 := cv_thread_mutex.cpp
all: $(TARGET) $(TARGET_2) $(TARGET_3) $(TARGET_4)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(CAIRO_FLAGS)

$(OBJ): $(SOURCE)
	$(CXX) $(CXXFLAGS) -c $< -o $@  $(CAIRO_FLAGS) 

$(TARGET_2): $(OBJ_2)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(OPENCV_FLAGS)

$(OBJ_2): $(SOURCE_2)
	$(CXX) $(CXXFLAGS) -c $< -o $@  $(OPENCV_FLAGS) 

$(TARGET_3): $(OBJ_3)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(OPENCV_FLAGS)

$(OBJ_3): $(SOURCE_3)
	$(CXX) $(CXXFLAGS) -c $< -o $@  $(OPENCV_FLAGS) 
	
$(TARGET_4): $(OBJ_4)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(OPENCV_FLAGS)

$(OBJ_4): $(SOURCE_4)
	$(CXX) $(CXXFLAGS) -c $< -o $@  $(OPENCV_FLAGS) 
clean:
	rm -f $(OBJ) $(TARGET)  $(OBJ_2) $(TARGET_2) $(OBJ_3) $(TARGET_3) $(OBJ_4) $(TARGET_4)
