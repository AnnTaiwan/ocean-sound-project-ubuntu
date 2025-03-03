#include <gst/gst.h>
#include <vector>
#include <iostream>
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
            std::cout << "\033[34m\033[1m----------------- receive gst message " << ++count_msg << " ----------------\033[0m" << std::endl;

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
            std::cout << "\033[34m\033[1m--------------END MSG-------------------\033[0m" << std::endl;
        }
    }

    return TRUE;
}

int main(int argc, char *argv[]) {
    // ############ Reading file ############ //
    std::cout << "Ready for Recording ?" << std::endl;
    
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
    source = gst_element_factory_make("alsasrc", "source");
    convert = gst_element_factory_make("audioconvert", "convert");
    resample = gst_element_factory_make("audioresample", "resample");
    spectrogram = gst_element_factory_make("spectrogram", "spectrogram");
    sink = gst_element_factory_make("fakesink", "sink"); 

    g_object_set (source, "device", "hw:1", NULL);

    if (!pipeline || !source || !convert || !resample || !spectrogram || !sink) {
        g_printerr("Not all gst elements could be created.\n");
        return -1;
    }

    // Build the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, convert, resample, spectrogram, sink, NULL);

    caps = gst_caps_new_simple("audio/x-raw", "format", G_TYPE_STRING, "F32LE", "channels", G_TYPE_INT, 1, "rate", G_TYPE_INT, 22050, NULL);

    if (!gst_element_link(source, convert) || 
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

