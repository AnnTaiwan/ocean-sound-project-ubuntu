#ifndef CAL_MEL_SPEC_H
#define CAL_MEL_SPEC_H
#include <iostream>
#include <vector>
#include <deque>
#include <cmath>
#include <fstream> // For file operations

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/audio/audio.h>
#include <gst/fft/gstfftf32.h>

#include <algorithm>
#include <stdexcept>
#include <numeric>
#include <limits>

#define SAMPLE_RATE 48000
#define DURATION 60.0
#define AUDIO_LEN (SAMPLE_RATE * DURATION)
#define N_MELS 512
#define N_FFT 4096
#define HOP_LEN 1024
#define SPEC_WIDTH ((AUDIO_LEN / HOP_LEN) + 1)
#define FMAX (SAMPLE_RATE / 2)
#define SPEC_SHAPE {SPEC_WIDTH, N_MELS}
#define CENTER true // used in STFT padded mode

std::vector<float> hanning_window(int window_Size);
std::vector<float> apply_hamming_window(const std::vector<float>& signal);
std::vector<double> fft_frequencies(float sr, int n_fft);
float hz_to_mel(float freq, bool htk = false);
float mel_to_hz(float mel, bool htk = false);
std::vector<double> mel_frequencies(int n_mels, float fmin, float fmax, bool htk);
void normalize(std::vector<std::vector<float>>& weights, const std::string& norm);
std::vector<std::vector<float>> generate_mel_filter_bank(int sr, int n_fft, int n_mels, float fmin, float fmax);
std::vector<std::vector<float>> power_to_db(
    const std::vector<std::vector<float>>& mel_spec, 
    float ref = 1.0f, 
    float amin = 1e-10f, 
    float top_db = 80.0f
);
std::vector<std::vector<GstFFTF32Complex>> compute_stft(
    const std::vector<float>& audio, 
    const std::vector<float>& window, 
    int n_fft, 
    int hop_length);
std::vector<std::vector<float>> compute_power_spectrogram(const std::vector<std::vector<GstFFTF32Complex>>& stft_matrix);
std::vector<std::vector<float>> apply_mel_filter_bank;
std::vector<std::vector<float>> get_mel_spectrogram(const std::vector<float>& audio, int sr);
void write_mel_spectrogram_to_txt(const std::vector<std::vector<float>>& mel_spec, const std::string& filename);
#endif

