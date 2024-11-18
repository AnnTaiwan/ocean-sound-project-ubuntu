CXX=${CXX:-g++}
GSTREAMER_FLAGS=$(pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-fft-1.0)
CAIRO_FLAGS=$(pkg-config --cflags --libs cairo)

$CXX -std=c++17 -O2 -o cal_mel_spec_ver4_for_ocean cal_mel_spec_ver4_for_ocean.cpp ${GSTREAMER_FLAGS}
$CXX -std=c++17 -O2 -o plot_mel_spec_from_txt_ver2_for_ocean plot_mel_spec_from_txt_ver2_for_ocean.cpp ${CAIRO_FLAGS}
