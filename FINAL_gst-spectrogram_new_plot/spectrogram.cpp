#include "spectrogram.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <time.h>
// for ploting image
#include <cairo.h>
#include <string>
// I add
#define PACKAGE "spectrogram_plugin"

GST_DEBUG_CATEGORY_STATIC(spectrogram_debug);
#define GST_CAT_DEFAULT spectrogram_debug

#define SAMPLE_RATE 22050
#define DURATION 3.0
#define SAMPLE_LEN (SAMPLE_RATE * DURATION)
#define N_MELS 64
#define N_FFT 1024
#define HOP_LEN 1024
#define SPEC_WIDTH (int(SAMPLE_LEN / HOP_LEN) + 1)
#define FMAX (SAMPLE_RATE / 2)
#define CENTER true // used in STFT padded mode
static int count_idx = 1; // number of the image

const int SPEC_SHAPE[2] = {N_MELS, SPEC_WIDTH};

G_DEFINE_TYPE(GstSpectrogram, gst_spectrogram, GST_TYPE_BASE_TRANSFORM);

// Declare pad templates as static templates
static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE(
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("audio/x-raw, format=(string)F32LE, channels=(int)1, rate=(int)22050"));

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE(
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("audio/x-raw, format=(string)F32LE, channels=(int)1, rate=(int)22050"));

// Forward declaration of the chain function
static GstFlowReturn spectrogram_transform_ip(GstBaseTransform *trans, GstBuffer *buffer);

// Function to initialize the filter class
static void gst_spectrogram_class_init(GstSpectrogramClass *klass) {
    GstElementClass *element_class = GST_ELEMENT_CLASS(klass);
    GstBaseTransformClass *transform_class = GST_BASE_TRANSFORM_CLASS(klass);

    // Set metadata for the element
    gst_element_class_set_metadata(
        element_class,
        "Get Spectrogram",             // long name
        "Filter/Audio",                       // classification
        "Filter to compute Mel Spectrogram",  // description
        "Your Name <youremail@example.com>"  // author
    );

    transform_class->transform_ip = spectrogram_transform_ip;

    // Add pad templates to the element
    gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&sink_template));
    gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&src_template));
}

// Function to initialize the filter instance
static void gst_spectrogram_init(GstSpectrogram *filter) {
    g_mutex_init (&filter->lock);
	filter->audio_buffer.reserve(1000000);
}

// Function to generate Hanning window
std::vector<float> hanning_window(int window_Size) {
    std::vector<float> window(window_Size);

    for (int i = 0; i < window_Size; i++) {
        window[i] = 0.5 - 0.5 * cos(2.0f * M_PI * i / (window_Size - 1));
    }

    return window;
}

// Helper function to convert frequency to Mel scale
float hz_to_mel(float freq, bool htk) {
    if (htk) {
        // HTK Mel scale: 2595 * log10(1 + freq / 700)
        return 2595.0f * std::log10(1.0f + freq / 700.0f);
    }
    else {
        // Slaney Mel scale
        const float f_min = 0.0f;
        const float f_sp = 200.0f / 3;
        const float min_log_hz = 1000.0f;
        const float min_log_mel = (min_log_hz - f_min) / f_sp;
        const float logstep = std::log(6.4f) / 27.0f;

        float mels = (freq - f_min) / f_sp;

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
    }
    else {
        // Slaney Mel scale
        const float f_min = 0.0f;
        const float f_sp = 200.0f / 3;
        const float min_log_hz = 1000.0f;
        const float min_log_mel = (min_log_hz - f_min) / f_sp;
        const float logstep = std::log(6.4f) / 27.0f;

        float freqs = f_min + f_sp * mel;

        if (mel >= min_log_mel) {
            freqs = min_log_hz * std::exp(logstep * (mel - min_log_mel));
        }

        return freqs;
    }
}

// Function to compute STFT
std::vector<std::vector<GstFFTF32Complex>> compute_stft(const std::vector<float>& audio, const std::vector<float>& window, int n_fft, int hop_length) {
    std::vector<std::vector<GstFFTF32Complex>> stft_result;

    // initial FFT from Gstreamer
    GstFFTF32* fft = gst_fft_f32_new(n_fft, false);

    // apply FFT
    for (int frame = 0; frame + n_fft < audio.size(); frame += hop_length) {
        std::vector<float> segment(audio.begin() + frame, audio.begin() + frame + n_fft);
        std::vector<float> windowed_segment(n_fft);

        for (int i = 0; i < n_fft; i++) {
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

    for (int i = 0; i < spec_width; i++) {
        for (int j = 0; j < N_FFT /2 + 1; ++j) {
            power_spec[j][i] = pow(stft_matrix[i][j].r, 2) + pow(stft_matrix[i][j].i, 2);
        }
    }

    return power_spec;
}

// Function to generate Mel filter bank
std::vector<std::vector<float>> generate_mel_filter_bank(int sr, int n_fft, int n_mels, float fmin, float fmax) {
    std::vector<std::vector<float>> mel_filter_bank(n_mels, std::vector<float>(n_fft / 2 + 1, 0.0f));

    // Compute mel points
    std::vector<float> mel_points(n_mels + 2);
    float mel_fmin = 2595.0f * std::log10(1.0f + fmin / 700.0f);
    float mel_fmax = 2595.0f * std::log10(1.0f + fmax / 700.0f);

    for (int i = 0; i < n_mels + 2; i++) {
        mel_points[i] = 700.0f * (std::pow(10.0f, (mel_fmin + (mel_fmax - mel_fmin) * i / (n_mels + 1)) / 2595.0f) - 1.0f);
    }

    // Convert Hz to FFT bin numbers
    std::vector<int> bin_points(n_mels + 2);
    for (int i = 0; i < n_mels + 2; i++) {
        bin_points[i] = static_cast<int>(std::floor((n_fft + 1) * mel_points[i] / sr));
    }

    // Create filters
    for (int i = 1; i <= n_mels; i++) {
        for (int j = bin_points[i - 1]; j < bin_points[i]; ++j) {
            mel_filter_bank[i - 1][j] = (j - bin_points[i - 1]) / static_cast<float>(bin_points[i] - bin_points[i - 1]);
        }

        for (int j = bin_points[i]; j < bin_points[i + 1]; ++j) {
            mel_filter_bank[i - 1][j] = (bin_points[i + 1] - j) / static_cast<float>(bin_points[i + 1] - bin_points[i]);
        }
    }

    return mel_filter_bank;
}

// Function to apply Mel filter bank to power spectrogram
std::vector<std::vector<float>> apply_mel_filter_bank(
    const std::vector<std::vector<float>>& power_spec, 
    const std::vector<std::vector<float>>& mel_filter_bank) {
    int n_mels = mel_filter_bank.size();
	int num_frames = power_spec[0].size();
    // (1025 , 216) and (128 , 1025)
    // std::cout << power_spec.size() << " " << power_spec[0].size()
    // << " " << mel_filter_bank.size() << " " << mel_filter_bank[0].size() << "\n";

    std::vector<std::vector<float>> mel_spec(n_mels, std::vector<float>(num_frames, 0.0f));

    for (int i = 0; i < n_mels; i++) {
        for (int j = 0; j < num_frames; ++j) {
            for (size_t k = 0; k < mel_filter_bank[i].size(); ++k) {
                mel_spec[i][j] += mel_filter_bank[i][k] * power_spec[k][j];
            }
        }
    }

    return mel_spec;
}

///////////////////////////////
// Function to convert power spectrogram to dB scale
// use global value as max to cal with top_db, this way can create a more correct picture
// Function to convert power spectrogram to dB scale
std::vector<std::vector<float>> power_to_db(const std::vector<std::vector<float>>& mel_spec, float amin = 1e-10f, float top_db = 80.0f) {
    int n_mels = mel_spec.size();
    int num_frames = mel_spec[0].size();
    float max_val = std::numeric_limits<float>::lowest();

    std::vector<std::vector<float>> log_mel_spec(n_mels, std::vector<float>(num_frames));

    for (int i = 0; i < n_mels; i++) {
        for (int j = 0; j < num_frames; ++j) {
            float value = std::max(mel_spec[i][j], amin);
            float db = 10.0f * std::log10(value);
            log_mel_spec[i][j] = db;
            if (db > max_val) {
                max_val = db;
            }
        }
    }

    float lower_bound = max_val - top_db;
    for (int i = 0; i < n_mels; i++) {
        for (int j = 0; j < num_frames; ++j) {
            log_mel_spec[i][j] = std::max(log_mel_spec[i][j], lower_bound);
        }
    }

    return log_mel_spec;
}

void calu_mel_spectrogram(const std::vector<float>& audio, std::vector<std::vector<float>>& spec) {
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
    }
    else {
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
    auto mel_filter_bank = generate_mel_filter_bank(SAMPLE_RATE, N_FFT, N_MELS, 0.0, FMAX);
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
    // std::cout << "Gel_mel_spectrogram's shape : (" << log_mel_spec.size() 
    // << " " << log_mel_spec[0].size() << ")\n";

    spec = log_mel_spec;
}

void plot_mel_spectrogram(const std::vector<std::vector<float>>& mel_spec, std::vector<std::vector<short>>& image_r, std::vector<std::vector<short>>& image_g, std::vector<std::vector<short>>& image_b) {
    int n_mels = mel_spec.size();
    int num_frames = mel_spec[0].size();

	// Find the min and max value for normalization
    float min_value = std::numeric_limits<float>::max();
    float max_value = std::numeric_limits<float>::lowest();

    for (const auto & row : mel_spec) {
        for (float value : row) {
            if (value < min_value) min_value = value;
            if (value > max_value) max_value = value;
        }
    }

	std::vector<std::vector<short>> color_r(n_mels, std::vector<short>(num_frames));
	std::vector<std::vector<short>> color_g(n_mels, std::vector<short>(num_frames));
	std::vector<std::vector<short>> color_b(n_mels, std::vector<short>(num_frames));
	
    // Draw the spectrogram
    for (int i = 0; i < n_mels; i++) {
        for (int j = 0; j < num_frames; j++) {
            float value = mel_spec[i][j];

            // Normalize to [0, 1]
            float normalized_value = std::max(0.0f, std::min(1.0f, (value - min_value) / (max_value - min_value)));
			int idx = std::max(0, std::min(int(normalized_value * 255), 255));

			// Viridis colormap implementation
			color_r[i][j] = viridis[idx * 3 + 0];
			color_g[i][j] = viridis[idx * 3 + 1];
			color_b[i][j] = viridis[idx * 3 + 2];
        }
    }

	image_r = color_r;
	image_g = color_g;
	image_b = color_b;
}

static GValue gst_new_array(const std::vector<std::vector<short>>& data) {
    GValue arr = G_VALUE_INIT;
    GValue val = G_VALUE_INIT;
    GValue list = G_VALUE_INIT;

    g_value_init(&arr, GST_TYPE_ARRAY);
    g_value_init(&val, G_TYPE_INT);
    
    for (int i = 0; i < SPEC_SHAPE[0]; i++) {
        g_value_init(&list, GST_TYPE_LIST);

        for (int j = 0; j < SPEC_SHAPE[1]; j++) {
            g_value_set_int(&val, data[i][j]);
            gst_value_list_append_value(&list, &val);
        }

        gst_value_array_append_value(&arr, &list);
        g_value_unset(&list);
    }

    g_value_unset(&val);

    return arr;
}

// Function to handle incoming data and process it
static GstFlowReturn spectrogram_transform_ip(GstBaseTransform *trans, GstBuffer *buffer) {
    GstSpectrogram *filter = GST_SPECTROGRAM(trans);
	GstMapInfo map;
    
	g_mutex_lock (&filter->lock);
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    // Convert audio data to float vector
    float *audio_samples = reinterpret_cast<float *>(map.data);
    // wrong version
        //std::vector<float> audio_data(map.data, map.data + map.size / sizeof(float));
    // I correct it into this:
    std::vector<float> audio_data(audio_samples, audio_samples + map.size / sizeof(float));
    
	filter->audio_buffer.insert(filter->audio_buffer.end(), audio_data.begin(), audio_data.end());

	while (filter->audio_buffer.size() >= SAMPLE_LEN) {
        double START_TIMING, END_TIMING;
		START_TIMING = clock();

		// Generate Mel spectrogram from the audio buffer
		std::vector<float> audio_segment(filter->audio_buffer.begin(), filter->audio_buffer.begin() + SAMPLE_LEN);
		std::vector<std::vector<float>> mel_spectrogram;
		std::vector<std::vector<short>> image_r, image_g, image_b;

		calu_mel_spectrogram(audio_segment, mel_spectrogram);
		plot_mel_spectrogram(mel_spectrogram, image_r, image_g, image_b);

        // Package data and Post message
        if (image_r.size() == SPEC_SHAPE[0] && image_r[0].size() == SPEC_SHAPE[1] &&
            image_g.size() == SPEC_SHAPE[0] && image_g[0].size() == SPEC_SHAPE[1] &&
            image_b.size() == SPEC_SHAPE[0] && image_b[0].size() == SPEC_SHAPE[1]
            ){
            END_TIMING = clock();
		    std::cout << "\033[33m\033[1mPlugin processing time : " << (END_TIMING - START_TIMING) / CLOCKS_PER_SEC << "s\033[0m" << std::endl;
		    // For ploting the image
		    int width = SPEC_SHAPE[1];
            int height = SPEC_SHAPE[0];
		    // Create a surface without extra margins
            cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
            cairo_t *cr = cairo_create(surface);
		    
		    // Draw the spectrogram
            for (int x = 0; x < SPEC_SHAPE[0]; ++x) {
                for (int y = 0; y < SPEC_SHAPE[1]; ++y) {
                    float r = (float)image_r[x][y] / 255;
                    float g = (float)image_g[x][y] / 255;
                    float b = (float)image_b[x][y] / 255;
                    if(x == 0 && y < 10)
                        std::cout << r << " " << g << " " << b << "\n";
                    cairo_set_source_rgb(cr, r, g, b);
                    cairo_rectangle(cr, y, height - x - 1, 1, 1); // Directly fit the data
                    cairo_fill(cr);
                }
            }
            std::string output_file_path = "mel_images/image_" + std::to_string(count_idx) + ".png";
            cairo_destroy(cr);
            cairo_surface_write_to_png(surface, output_file_path.c_str());  // Save as PNG
            cairo_surface_destroy(surface);
            std::cout << "Successfully Plot " << output_file_path << std::endl;
            count_idx++;
            GstStructure *s;
            /*
            GValue array_r = gst_new_array(image_r);
            GValue array_g = gst_new_array(image_g);
            GValue array_b = gst_new_array(image_b);

            s = gst_structure_new("spectrogram", NULL);

            gst_structure_set_value (s, "array_r", &array_r);
            gst_structure_set_value (s, "array_g", &array_g);
            gst_structure_set_value (s, "array_b", &array_b);

            END_TIMING = clock();
		    std::cout << "\033[33m\033[1mPlugin processing time : " << (END_TIMING - START_TIMING) / CLOCKS_PER_SEC << "s\033[0m" << std::endl;

            std::cout << "\033[33m\033[1mPost message with image data !\033[0m\n" << std::endl;
            GstMessage *message = gst_message_new_element(GST_OBJECT(filter), s);
            gst_element_post_message(GST_ELEMENT(filter), message);

            g_value_unset(&array_r);
            g_value_unset(&array_g);
            g_value_unset(&array_b);
            */
        }
        else {
            END_TIMING = clock();
		    std::cout << "\033[33m\033[1mPlugin processing time : " << (END_TIMING - START_TIMING) / CLOCKS_PER_SEC << "s\033[0m" << std::endl;

            std::cout << "\033[31m\033[1mNot correct output size of the spectrogram\033[0m" << std::endl;
        }

		filter->audio_buffer.erase(filter->audio_buffer.begin(), filter->audio_buffer.begin() + SAMPLE_LEN);
	}

    // Unmap the buffer
    gst_buffer_unmap(buffer, &map);
	g_mutex_unlock (&filter->lock);

    return GST_FLOW_OK;
}

// Function to initialize the plugin
static gboolean plugin_init(GstPlugin *plugin) {
    GST_DEBUG_CATEGORY_INIT(spectrogram_debug, "spectrogram", 0, "Plugin to compute Mel spectrogram");

    return gst_element_register(plugin, "spectrogram", GST_RANK_NONE, GST_TYPE_SPECTROGRAM);
}

/*
// Define the plugin structure
GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    spectrogram,                     // Plugin name
    "Plugin to compute Mel spectrogram", // Plugin description
    plugin_init,                         // Plugin initialization function
    GST_PACKAGE_VERSION, 
    GST_PACKAGE_LICENSE, 
    GST_PACKAGE_NAME, 
    GST_PACKAGE_ORIGIN
)
*/
// Define the plugin structure
GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    spectrogram,                     // Plugin name
    "Plugin to compute Mel spectrogram", // Plugin description
    plugin_init,                         // Plugin initialization function
    "1.0.0",                             // Plugin version
    "LGPL",                              // Plugin license
    "Mel Spectrogram",                   // Plugin package name
    "https://example.com/mel_spectrogram" // Plugin source URL
)
