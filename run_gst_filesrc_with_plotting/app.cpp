#include <gst/gst.h>
#include <vector>
#include <iostream>

// for ploting image
#include <cairo.h>
#include <string>

#define SAMPLE_RATE 22050
#define DURATION 3.0
#define SAMPLE_LEN (SAMPLE_RATE * DURATION)
#define N_MELS 64
#define N_FFT 1024
#define HOP_LEN 1024
#define SPEC_WIDTH (int(SAMPLE_LEN / HOP_LEN) + 1)
#define FMAX (SAMPLE_RATE / 2)
#define CENTER true // used in STFT padded mode

const int SPEC_SHAPE[2] = {N_MELS, SPEC_WIDTH};


#define BUS_WIDTH  16
#define DATA_WIDTH 2
#define DATA_DEPTH (BUS_WIDTH / DATA_WIDTH)

#define MEM_SIZE (1024 * 1024 * 16)
#define PAGE_SIZE 4096

#define IMAGE_SIZE 64
#define CLASS_NUM 4

#define TILE_SIZE 32
#define BITS_OFFSET 0  // 8 - 8

// I add
static int count_msg = 0;
static int count_idx = 0;

std::vector<std::vector<int16_t>> gst_get_array(const GValue* arr) {
    std::vector<std::vector<int16_t>> result;

    if (GST_VALUE_HOLDS_ARRAY(arr)) {
        for (gsize i = 0; i < gst_value_array_get_size(arr); i++) {
            const GValue* list = gst_value_array_get_value(arr, i);

            std::vector<int16_t> row;

            if (GST_VALUE_HOLDS_LIST(list)) {
                for (gsize j = 0; j < gst_value_list_get_size(list); j++) {
                    const GValue* val = gst_value_list_get_value(list, j);

                    if (G_VALUE_HOLDS_INT(val)) {
                        int16_t data = g_value_get_int(val);
                        row.push_back(data);
                    }
                    else {
                        std::cerr << "Error: List element is not G_TYPE_INT!" << std::endl;
                    }
                }
            }
            else {
                std::cerr << "Error: Array element is not GST_TYPE_LIST!" << std::endl;
            }

            result.push_back(row);
        }
    }
    else {
        std::cerr << "Error: Input is not a GST_TYPE_ARRAY!" << std::endl;
    }

    return result;
}
static gboolean message_handler(GstBus * bus, GstMessage * msg, gpointer data) {

    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ELEMENT) {
        const GstStructure *s = gst_message_get_structure (msg);
        const gchar *name = gst_structure_get_name (s);

        if (strcmp (name, "spectrogram") == 0) {
            std::cout << "\033[34m\033[1m----------------- receive gst message " << ++count_msg << " ----------------\n\033[0m" << std::endl;

            std::vector<std::vector<int16_t>> image_r = gst_get_array(gst_structure_get_value (s, "array_r"));
            std::vector<std::vector<int16_t>> image_g = gst_get_array(gst_structure_get_value (s, "array_g"));
            std::vector<std::vector<int16_t>> image_b = gst_get_array(gst_structure_get_value (s, "array_b"));
            
            std::cout << "image_r: ";
            for(int i = 50; i < 60; i++){

                std::cout << image_r[0][i] << " ";
            }
            std::cout << std::endl;

            std::cout << "image_g: ";
            for(int i = 50; i < 60; i++){

                std::cout << image_g[0][i] << " ";
            }
            std::cout << std::endl;
            
            std::cout << "image_b: ";
            for(int i = 50; i < 60; i++){

                std::cout << image_b[0][i] << " ";
            }
            std::cout << std::endl;
            std::cout << "\033[34m\033[1m----------------END MSG---------------\n\033[0m" << std::endl;

            /* plotting picture for testing */
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
                        std::cout << "RGB: " <<  r << " " << g << " " << b << "\n";
                    cairo_set_source_rgb(cr, r, g, b);
                    cairo_rectangle(cr, y, height - x - 1, 1, 1); // Directly fit the data
                    cairo_fill(cr);
                }
            }
            std::string output_file_path = "mel_images/Mel_Image_" + std::to_string(count_idx) + ".png";
            cairo_destroy(cr);
            cairo_surface_write_to_png(surface, output_file_path.c_str());  // Save as PNG
            cairo_surface_destroy(surface);
            std::cout << "Successfully Plot " << output_file_path << std::endl;
            count_idx++;
            /* end testing */
        }
    }

    return TRUE;
}

int main(int argc, char *argv[]) {
    // ############ Reading file ############ //
    std::cout << "Reading file ......" << std::endl;

    if(argc != 2) {
        std::cout << "\033[31mYour argc is " << argc << ", the correct argc should be 2(./app <audio_filename>)\033[0m" << std::endl;
        return -1;
    }
    
    // ############ Setting gstreamer ############ //
    std::cout << "Setting gstreamer ......" << std::endl;

    GstElement *pipeline;
    GstElement *source, *decode, *convert, *resample, *spectrogram, *sink;
    GstBus *bus;
    GstCaps *caps;
    GstMessage *msg;
    GstStateChangeReturn ret;
    GMainLoop *loop;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create the empty pipeline
    pipeline = gst_pipeline_new("pipeline");

    // Create the elements
    source = gst_element_factory_make("filesrc", "source");
    decode = gst_element_factory_make("wavparse", "decode");
    convert = gst_element_factory_make("audioconvert", "convert");
    resample = gst_element_factory_make("audioresample", "resample");
    spectrogram = gst_element_factory_make("spectrogram", "spectrogram");
    sink = gst_element_factory_make("fakesink", "sink"); 

    g_object_set(source, "location", argv[1], NULL);

    if (!pipeline || !source || !decode || !convert || !resample || !spectrogram || !sink) {
        g_printerr("Not all gst elements could be created.\n");
        return -1;
    }

    // Build the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, decode, convert, resample, spectrogram, sink, NULL);

    caps = gst_caps_new_simple("audio/x-raw", "format", G_TYPE_STRING, "F32LE", "channels", G_TYPE_INT, 1, "rate", G_TYPE_INT, 22050, NULL);

    if (!gst_element_link(source, decode) || 
        !gst_element_link(decode, convert) || 
        !gst_element_link(convert, resample) || 
        !gst_element_link_filtered(resample, spectrogram, caps) || 
        !gst_element_link(spectrogram, sink)
        ) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    gst_caps_unref(caps);

    // ############ Running gstreamer ############ //
    std::cout << "Running gstreamer ....." << std::endl;

    bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, message_handler, NULL);
    gst_object_unref(bus);

    // Start playing
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // ############ Running begin ############ //
    std::cout << "Running begin ......" << std::endl;
    std::cout << "\033[31m\033[1m------------------------------------------------------------\033[0m" << std::endl;

    // we need to run a GLib main loop to get the messages
    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run(loop);

    // ############# Running end ############# //
    std::cout << "\033[31m\033[1m------------------------------------------------------------\033[0m" << std::endl;
    std::cout << "Running stop ......" << std::endl;

    // Stop playing
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}

