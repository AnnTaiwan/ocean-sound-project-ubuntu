#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <cmath>
#include <cairo.h>

#define SAMPLE_RATE 48000
#define DURATION 60.0
#define AUDIO_LEN (SAMPLE_RATE * DURATION)
#define N_MELS 512
#define N_FFT 4096
#define HOP_LEN 1024
#define SPEC_WIDTH 2813 // (AUDIO_LEN / HOP_LEN + 1)
#define FMAX (SAMPLE_RATE / 2)
#define SPEC_SHAPE {SPEC_WIDTH, N_MELS}
#define CENTER true // used in STFT padded mode

// Function to read Mel spectrogram from .txt file
std::vector<std::vector<float>> read_mel_spectrogram_from_txt(const std::string& file_path, int num_mels, int num_frames) {
    std::ifstream file(file_path);
    std::vector<std::vector<float>> mel_spec(num_frames, std::vector<float>(num_mels)); // Note the transposition here

    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << file_path << std::endl;
        return mel_spec;
    }

    std::string line;
    int mel = 0;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        float value;
        int frame = 0;
        while (iss >> value) {
            mel_spec[frame++][mel] = value;
        }
        ++mel;
        //if (frame >= num_frames) break;  // Stop if we have read enough frames
    }

    file.close();
    return mel_spec;
}

// Viridis colormap implementation
void get_color_from_viridis(float value, float& r, float& g, float& b) {
    const int N = 256;
    static const double viridis[256][3] = 
    {
      68,1,84,
      68,2,85,
      68,3,87,
      69,5,88,
      69,6,90,
      69,8,91,
      70,9,92,
      70,11,94,
      70,12,95,
      70,14,97,
      71,15,98,
      71,17,99,
      71,18,101,
      71,20,102,
      71,21,103,
      71,22,105,
      71,24,106,
      72,25,107,
      72,26,108,
      72,28,110,
      72,29,111,
      72,30,112,
      72,32,113,
      72,33,114,
      72,34,115,
      72,35,116,
      71,37,117,
      71,38,118,
      71,39,119,
      71,40,120,
      71,42,121,
      71,43,122,
      71,44,123,
      70,45,124,
      70,47,124,
      70,48,125,
      70,49,126,
      69,50,127,
      69,52,127,
      69,53,128,
      69,54,129,
      68,55,129,
      68,57,130,
      67,58,131,
      67,59,131,
      67,60,132,
      66,61,132,
      66,62,133,
      66,64,133,
      65,65,134,
      65,66,134,
      64,67,135,
      64,68,135,
      63,69,135,
      63,71,136,
      62,72,136,
      62,73,137,
      61,74,137,
      61,75,137,
      61,76,137,
      60,77,138,
      60,78,138,
      59,80,138,
      59,81,138,
      58,82,139,
      58,83,139,
      57,84,139,
      57,85,139,
      56,86,139,
      56,87,140,
      55,88,140,
      55,89,140,
      54,90,140,
      54,91,140,
      53,92,140,
      53,93,140,
      52,94,141,
      52,95,141,
      51,96,141,
      51,97,141,
      50,98,141,
      50,99,141,
      49,100,141,
      49,101,141,
      49,102,141,
      48,103,141,
      48,104,141,
      47,105,141,
      47,106,141,
      46,107,142,
      46,108,142,
      46,109,142,
      45,110,142,
      45,111,142,
      44,112,142,
      44,113,142,
      44,114,142,
      43,115,142,
      43,116,142,
      42,117,142,
      42,118,142,
      42,119,142,
      41,120,142,
      41,121,142,
      40,122,142,
      40,122,142,
      40,123,142,
      39,124,142,
      39,125,142,
      39,126,142,
      38,127,142,
      38,128,142,
      38,129,142,
      37,130,142,
      37,131,141,
      36,132,141,
      36,133,141,
      36,134,141,
      35,135,141,
      35,136,141,
      35,137,141,
      34,137,141,
      34,138,141,
      34,139,141,
      33,140,141,
      33,141,140,
      33,142,140,
      32,143,140,
      32,144,140,
      32,145,140,
      31,146,140,
      31,147,139,
      31,148,139,
      31,149,139,
      31,150,139,
      30,151,138,
      30,152,138,
      30,153,138,
      30,153,138,
      30,154,137,
      30,155,137,
      30,156,137,
      30,157,136,
      30,158,136,
      30,159,136,
      30,160,135,
      31,161,135,
      31,162,134,
      31,163,134,
      32,164,133,
      32,165,133,
      33,166,133,
      33,167,132,
      34,167,132,
      35,168,131,
      35,169,130,
      36,170,130,
      37,171,129,
      38,172,129,
      39,173,128,
      40,174,127,
      41,175,127,
      42,176,126,
      43,177,125,
      44,177,125,
      46,178,124,
      47,179,123,
      48,180,122,
      50,181,122,
      51,182,121,
      53,183,120,
      54,184,119,
      56,185,118,
      57,185,118,
      59,186,117,
      61,187,116,
      62,188,115,
      64,189,114,
      66,190,113,
      68,190,112,
      69,191,111,
      71,192,110,
      73,193,109,
      75,194,108,
      77,194,107,
      79,195,105,
      81,196,104,
      83,197,103,
      85,198,102,
      87,198,101,
      89,199,100,
      91,200,98,
      94,201,97,
      96,201,96,
      98,202,95,
      100,203,93,
      103,204,92,
      105,204,91,
      107,205,89,
      109,206,88,
      112,206,86,
      114,207,85,
      116,208,84,
      119,208,82,
      121,209,81,
      124,210,79,
      126,210,78,
      129,211,76,
      131,211,75,
      134,212,73,
      136,213,71,
      139,213,70,
      141,214,68,
      144,214,67,
      146,215,65,
      149,215,63,
      151,216,62,
      154,216,60,
      157,217,58,
      159,217,56,
      162,218,55,
      165,218,53,
      167,219,51,
      170,219,50,
      173,220,48,
      175,220,46,
      178,221,44,
      181,221,43,
      183,221,41,
      186,222,39,
      189,222,38,
      191,223,36,
      194,223,34,
      197,223,33,
      199,224,31,
      202,224,30,
      205,224,29,
      207,225,28,
      210,225,27,
      212,225,26,
      215,226,25,
      218,226,24,
      220,226,24,
      223,227,24,
      225,227,24,
      228,227,24,
      231,228,25,
      233,228,25,
      236,228,26,
      238,229,27,
      241,229,28,
      243,229,30,
      246,230,31,
      248,230,33,
      250,230,34,
      253,231,36,
    };
	
    int idx = std::min(static_cast<int>(value * (N - 1)), N - 1);
    r = viridis[idx][0] / 255;
    g = viridis[idx][1] / 255;
    b = viridis[idx][2] / 255;
}
void plot_mel_spectrogram(const std::vector<std::vector<float>>& mel_spec, const std::string& output_file) {
    int num_frames = mel_spec.size();
    int num_mels = mel_spec[0].size();
    int width = num_frames;
    int height = num_mels;

    // Create a surface without extra margins
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);

    // Find the min and max value for normalization
    float min_value = std::numeric_limits<float>::max();
    float max_value = std::numeric_limits<float>::lowest();
    for (const auto& row : mel_spec) {
        for (float value : row) {
            if (value < min_value) min_value = value;
            if (value > max_value) max_value = value;
        }
    }

    // Draw the spectrogram
    for (int y = 0; y < num_mels; ++y) {
        for (int x = 0; x < num_frames; ++x) {
            float value = mel_spec[x][y];
            // Normalize to [0, 1]
            float normalized_value = (value - min_value) / (max_value - min_value);

            // Clip the normalized value to ensure it's within [0, 1]
            normalized_value = std::max(0.0f, std::min(1.0f, normalized_value));

            float r, g, b;
            get_color_from_viridis(normalized_value, r, g, b);
            cairo_set_source_rgb(cr, r, g, b);
            cairo_rectangle(cr, x, height - y - 1, 1, 1); // Directly fit the data
            cairo_fill(cr);
        }
    }

    cairo_destroy(cr);
    cairo_surface_write_to_png(surface, output_file.c_str());  // Save as PNG
    cairo_surface_destroy(surface);
}
int main(int argc, char *argv[]) {
    if(argc < 3) {
        std::cout << "Error: Please add the txt files after the execution command, like ./plot_mel_spec_from_txt <files.txt> <image.png>" << std::endl;
        return 0;
    }
    std::string file_path = argv[1];
    int num_mels = N_MELS;  // Number of Mel bands
    int num_frames = SPEC_WIDTH;  // Number of frames

    std::string dest_image_path = argv[2];
    std::vector<std::vector<float>> mel_spec = read_mel_spectrogram_from_txt(file_path, num_mels, num_frames);
    plot_mel_spectrogram(mel_spec, dest_image_path);

    return 0;
}
