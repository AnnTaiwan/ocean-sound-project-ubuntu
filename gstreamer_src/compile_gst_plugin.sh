#!/bin/bash

# Use g++ as the default compiler if CXX is not set
CXX=${CXX:-g++}  

# Gather the necessary flags from pkg-config
GSTREAMER_FLAGS=$(pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-fft-1.0)
CAIRO_FLAGS=$(pkg-config --cflags --libs cairo)

# Include directory for header files (adjust if headers are located elsewhere)
INCLUDE_DIR="./"

# Compile cal_mel_spec.cpp into position-independent code
$CXX -std=c++17 -O2 -fPIC -c cal_mel_spec.cpp -o cal_mel_spec.o ${GSTREAMER_FLAGS} ${CAIRO_FLAGS} -I${INCLUDE_DIR}

# Compile and link gst_cal_spec_plugin.cpp into a shared library
$CXX -std=c++17 -O2 -shared -fPIC -o mel_spectrogram.so \
    mel_spectrogram.cpp cal_mel_spec.o ${GSTREAMER_FLAGS} ${CAIRO_FLAGS} -I${INCLUDE_DIR}
    
