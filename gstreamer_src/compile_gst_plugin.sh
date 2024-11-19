CXX=${CXX:-g++}  # Use g++ as the default compiler if CXX is not set
GSTREAMER_FLAGS=$(pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-fft-1.0)
CAIRO_FLAGS=$(pkg-config --cflags --libs cairo)

# Include directory for the header file (assuming it's in the current directory)
INCLUDE_DIR="./"  # Update this if the header file is in a different directory

# Compile the source file into a shared library
$CXX -std=c++17 -O2 -o gst_cal_spec_plugin.so -shared -fPIC \
    gst_cal_spec_plugin.cpp ${GSTREAMER_FLAGS} ${CAIRO_FLAGS} -I${INCLUDE_DIR}

