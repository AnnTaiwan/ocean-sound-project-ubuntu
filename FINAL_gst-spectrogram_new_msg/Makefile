CXX := g++
CXXFLAGS := -std=c++17 -O2 -fPIC

# Use pkg-config to get necessary GStreamer and Cairo flags
GSTREAMER_FLAGS := $(shell pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-fft-1.0)
CAIRO_FLAGS := $(shell pkg-config --cflags --libs cairo)

# Include directory
INCLUDE_DIR := ./

# Target file
TARGET := spectrogram.so
OBJ := spectrogram.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(GSTREAMER_FLAGS) $(CAIRO_FLAGS)

$(OBJ): spectrogram.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(GSTREAMER_FLAGS) $(CAIRO_FLAGS) -I$(INCLUDE_DIR)

clean:
	rm -f $(OBJ) $(TARGET)

