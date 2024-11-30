#include "cal_mel_spec.h"
// Function to generate Hanning window
std::vector<float> hanning_window(int window_Size) {
    std::vector<float> window(window_Size);
    for (int i = 0; i < window_Size; ++i) {
        window[i] = 0.5 - 0.5 * cos(2.0f * M_PI * i / (window_Size - 1));
    }
    return window;
}

// Helper function to convert frequency to Mel scale
float hz_to_mel(float freq, bool htk) {
    if (htk) {
        // HTK Mel scale: 2595 * log10(1 + freq / 700)
        return 2595.0f * std::log10(1.0f + freq / 700.0f);
    } else {
        // Slaney Mel scale
        const float f_min = 0.0f;
        const float f_sp = 200.0f / 3;
        float mels = (freq - f_min) / f_sp;
        const float min_log_hz = 1000.0f;
        const float min_log_mel = (min_log_hz - f_min) / f_sp;
        const float logstep = std::log(6.4f) / 27.0f;

        if (freq >= min_log_hz) {
            mels = min_log_mel + std::log(freq / min_log_hz) / logstep;
        }

        return mels;
    }
}

// Helper function to convert Mel scale to frequency
float mel_to_hz(float mel, bool htk) {
    if (htk) {
        // HTK Mel scale: 700 * (10^(mel / 2595) - 1)
        return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
    } else {
        // Slaney Mel scale
        const float f_min = 0.0f;
        const float f_sp = 200.0f / 3;
        float freqs = f_min + f_sp * mel;
        const float min_log_hz = 1000.0f;
        const float min_log_mel = (min_log_hz - f_min) / f_sp;
        const float logstep = std::log(6.4f) / 27.0f;

        if (mel >= min_log_mel) {
            freqs = min_log_hz * std::exp(logstep * (mel - min_log_mel));
        }

        return freqs;
    }
}

// Function to create Mel filter-bank
// Function to generate Mel filter bank
std::vector<std::vector<float>> generate_mel_filter_bank(int sr, int n_fft, int n_mels, float fmin, float fmax) {
    std::vector<std::vector<float>> mel_filter_bank(n_mels, std::vector<float>(n_fft / 2 + 1, 0.0f));

    // Compute mel points
    std::vector<float> mel_points(n_mels + 2);
    float mel_fmin = 2595.0f * std::log10(1.0f + fmin / 700.0f);
    float mel_fmax = 2595.0f * std::log10(1.0f + fmax / 700.0f);

    for (int i = 0; i < n_mels + 2; ++i) {
        mel_points[i] = 700.0f * (std::pow(10.0f, (mel_fmin + (mel_fmax - mel_fmin) * i / (n_mels + 1)) / 2595.0f) - 1.0f);
    }

    // Convert Hz to FFT bin numbers
    std::vector<int> bin_points(n_mels + 2);
    for (int i = 0; i < n_mels + 2; ++i) {
        bin_points[i] = static_cast<int>(std::floor((n_fft + 1) * mel_points[i] / sr));
    }

    // Create filters
    for (int i = 1; i <= n_mels; ++i) {
        for (int j = bin_points[i - 1]; j < bin_points[i]; ++j) {
            mel_filter_bank[i - 1][j] = (j - bin_points[i - 1]) / static_cast<float>(bin_points[i] - bin_points[i - 1]);
        }
        for (int j = bin_points[i]; j < bin_points[i + 1]; ++j) {
            mel_filter_bank[i - 1][j] = (bin_points[i + 1] - j) / static_cast<float>(bin_points[i + 1] - bin_points[i]);
        }
    }

    return mel_filter_bank;
}

///////////////////////////////
// Function to convert power spectrogram to dB scale
// use global value as max to cal with top_db, this way can create a more correct picture
// Function to convert power spectrogram to dB scale
std::vector<std::vector<float>> power_to_db(
    const std::vector<std::vector<float>>& mel_spec, 
    float ref, 
    float amin, 
    float top_db
) {
    int num_frames = mel_spec.size();
    int n_mels = mel_spec[0].size();

    std::vector<std::vector<float>> log_mel_spec(num_frames, std::vector<float>(n_mels));

    float max_val = -std::numeric_limits<float>::infinity();

    for (int i = 0; i < num_frames; ++i) {
        for (int j = 0; j < n_mels; ++j) {
            float value = std::max(mel_spec[i][j], amin);
            float db = 10.0f * std::log10(value);
            log_mel_spec[i][j] = db;
            if (db > max_val) {
                max_val = db;
            }
        }
    }

    float lower_bound = max_val - top_db;
    for (int i = 0; i < num_frames; ++i) {
        for (int j = 0; j < n_mels; ++j) {
            log_mel_spec[i][j] = std::max(log_mel_spec[i][j], lower_bound);
        }
    }

    return log_mel_spec;
}

// Function to compute STFT
std::vector<std::vector<GstFFTF32Complex>> compute_stft(
    const std::vector<float>& audio, 
    const std::vector<float>& window, 
    int n_fft, 
    int hop_length) {
    std::vector<std::vector<GstFFTF32Complex>> stft_result;

    // initial FFT from Gstreamer
    GstFFTF32* fft = gst_fft_f32_new(n_fft, false);

    // apply FFT
    for (int frame = 0; frame + n_fft < audio.size(); frame += hop_length) {
        std::vector<float> segment(audio.begin() + frame, audio.begin() + frame + n_fft);
        std::vector<float> windowed_segment(n_fft);
        for (int i = 0; i < n_fft; ++i) {
            windowed_segment[i] = segment[i] * window[i];
        }

        GstFFTF32Complex out_fft[n_fft];
        gst_fft_f32_fft(fft, windowed_segment.data(), out_fft);

        std::vector<GstFFTF32Complex> frame_fft(out_fft, out_fft + n_fft / 2 + 1);
        stft_result.push_back(frame_fft);
    }

    gst_fft_f32_free(fft);

    return stft_result;
}

// Function to compute power spectrogram
std::vector<std::vector<float>> compute_power_spectrogram(const std::vector<std::vector<GstFFTF32Complex>>& stft_matrix) {
    int spec_height = stft_matrix[0].size();
    int spec_width = stft_matrix.size();

    std::vector<std::vector<float>> power_spec(spec_height, std::vector<float>(spec_width, 0.0f));

    for (int i = 0; i < spec_width; ++i) {
        for (int j = 0; j < N_FFT /2 + 1; ++j) {
            power_spec[j][i] = pow(stft_matrix[i][j].r, 2) + pow(stft_matrix[i][j].i, 2);
        }
    }
    return power_spec;
}


// Function to apply Mel filter bank to power spectrogram
std::vector<std::vector<float>> apply_mel_filter_bank(
    const std::vector<std::vector<float>>& power_spec, 
    const std::vector<std::vector<float>>& mel_filter_bank
) {
    int num_frames = power_spec[0].size();
    int n_mels = mel_filter_bank.size();
    // (1025 , 216) and (128 , 1025)
    // std:: cout << power_spec.size() << " " << power_spec[0].size()
    // << " " << mel_filter_bank.size() << " " << mel_filter_bank[0].size() << "\n";

    std::vector<std::vector<float>> mel_spec(n_mels, std::vector<float>(num_frames, 0.0f));

    for (int i = 0; i < n_mels; ++i) {
        for (int j = 0; j < num_frames; ++j) {
            for (size_t k = 0; k < mel_filter_bank[i].size(); ++k) {
                mel_spec[i][j] += mel_filter_bank[i][k] * power_spec[k][j];
            }
        }
    }

    return mel_spec;
}

std::vector<std::vector<float>> get_mel_spectrogram(const std::vector<float>& audio, int sr) {
    guint fft_size = N_FFT;
    guint num_fft_bins = N_MELS;

    // Calculate padding length if CENTER mode is enabled
    int paddingLength = 0;
    std::vector<float> paddedSignal;
    if (CENTER) {
        int pad_Length = fft_size / 2;
        int audioLength = audio.size();

        // Create a new padded signal vector with the required padding
        paddedSignal.resize(pad_Length + audioLength + pad_Length, 0.0f);

        // Copy the original audio data into the new padded signal vector
        std::copy(audio.begin(), audio.end(), paddedSignal.begin() + pad_Length);
        } else {
            // If no padding is required, just copy the original signal
            paddedSignal = audio;
    }

    // Create a Hanning window of appropriate length
    std::vector<float> window(fft_size);
    window = hanning_window(fft_size);

    // Compute STFT
    auto stft_matrix = compute_stft(paddedSignal, window, N_FFT, HOP_LEN);
    // std::cout << "stft_matrix: " << stft_matrix.size() 
    // << " " << stft_matrix[0].size() << "\n"; // (216, 1025)

    // Compute power spectrogram
    auto power_spec = compute_power_spectrogram(stft_matrix);
    // std::cout << "power_spec: " << power_spec.size() 
    // << " " << power_spec[0].size() << "\n"; // (1025 , 216)

    // Generate Mel filter bank
    auto mel_filter_bank = generate_mel_filter_bank(sr, N_FFT, N_MELS, 0.0, FMAX);
    // std::cout << "mel_filter_bank: " << mel_filter_bank.size() 
    // << " " << mel_filter_bank[0].size() << "\n"; // (128, 1025)

    // Prepare Mel spectrogram container
    std::vector<std::vector<float>> mel_spec(N_MELS, std::vector<float>(SPEC_WIDTH, 0.0f));
    // Apply Mel filter bank
    mel_spec = apply_mel_filter_bank(power_spec, mel_filter_bank);
    // std::cout << "mel_spec: " << mel_spec.size() 
    // << " " << mel_spec[0].size() << "\n";

    // Convert to dB scale
    auto log_mel_spec = power_to_db(mel_spec);
    // (128, 216) in result
    // std::cout << "Gel_mel_spectrogram's shape : (" << log_mel_spec.size() << " " 
    //           << log_mel_spec[0].size() << ")\n";

    return log_mel_spec;
}

// Function to write Mel spectrogram to a .txt file
void write_mel_spectrogram_to_txt(const std::vector<std::vector<float>>& mel_spec, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (size_t i = 0; i < mel_spec.size(); ++i) {
            for (size_t j =0; j < SPEC_WIDTH; j++){
                file << mel_spec[i][j] << " ";
            }
            file << "\n";
        }
        
        file.close();
    } else {
        std::cerr << "Unable to open file " << filename << " for writing." << std::endl;
    }
    
}

