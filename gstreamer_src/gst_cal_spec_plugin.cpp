#include "cal_mel_spec.h"
#include <gst/gst.h>
#include <gst/gstelement.h>

#include <vector>
#include <deque>
#include <fstream>
#include <cmath>
#include <algorithm>

#define PLUGIN_NAME "mel_spectrogram"
#define PACKAGE "mel_spectrogram_plugin"
#define GST_TYPE_MEL_SPEC_FILTER (mel_spec_filter_get_type())
#define MEL_SPEC_FILTER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_MEL_SPEC_FILTER, MelSpecFilter))

// Plugin data structure
typedef struct _MelSpecFilter {
    GstElement parent;
    GstPad *sinkpad, *srcpad;
    std::deque<float> audio_buffer;
    gboolean silent;
} MelSpecFilter;

// Class structure for MelSpecFilter
typedef struct _MelSpecFilterClass {
    GstElementClass parent_class;
} MelSpecFilterClass;

G_DEFINE_TYPE(MelSpecFilter, mel_spec_filter, GST_TYPE_ELEMENT);

// Declare pad templates as static templates
static GstPadTemplate *sinkpad_template = NULL;
static GstPadTemplate *srcpad_template = NULL;

// Forward declaration of the chain function
static GstFlowReturn mel_spec_chain(GstPad *pad, GstObject *parent, GstBuffer *buffer);

// Function to initialize the filter class
static void mel_spec_filter_class_init(MelSpecFilterClass *klass) {
    GstElementClass *element_class = GST_ELEMENT_CLASS(klass);

    // Define the pad templates
    sinkpad_template = gst_pad_template_new("sink", GST_PAD_SINK, GST_PAD_ALWAYS, gst_caps_new_simple("application/x-raw", "format", G_TYPE_STRING, "S16LE", NULL));
    srcpad_template = gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS, gst_caps_new_simple("application/x-raw", "format", G_TYPE_STRING, "S16LE", NULL));

    // Set up pads for the element
    gst_element_class_add_pad_template(element_class,
        sinkpad_template);
    gst_element_class_add_pad_template(element_class,
        srcpad_template);
}

// Function to initialize the filter instance
static void mel_spec_filter_init(MelSpecFilter *filter) {
    filter->sinkpad = gst_pad_new_from_template(sinkpad_template, "sink");
    filter->srcpad = gst_pad_new_from_template(srcpad_template, "src");

    // Set the chain function for sinkpad
    gst_pad_set_chain_function(filter->sinkpad, mel_spec_chain);

    // Add pads to the element
    gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);
    gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);
}

// Function to handle incoming data and process it
static GstFlowReturn mel_spec_chain(GstPad *pad, GstObject *parent, GstBuffer *buffer) {
    MelSpecFilter *filter = MEL_SPEC_FILTER(parent);

    // Extract audio data from the buffer
    GstMapInfo info;
    gst_buffer_map(buffer, &info, GST_MAP_READ);
    std::vector<float> audio_data(info.data, info.data + info.size);

    // Generate Mel spectrogram from the audio buffer
    // Assuming get_mel_spectrogram accepts a std::vector of floats or modifies it
    std::vector<float> audio_data_copy(filter->audio_buffer.begin(), filter->audio_buffer.end());
    std::vector<std::vector<float>> mel_spectrogram = get_mel_spectrogram(audio_data_copy, SAMPLE_RATE);

    // Optionally, you can process the Mel spectrogram further or pass it to another element
    std::string filename = "mel_spec_temp.txt";
    write_mel_spectrogram_to_txt(mel_spectrogram, filename);

    // Unmap the buffer
    gst_buffer_unmap(buffer, &info);

    return GST_FLOW_OK;
}

// Function to initialize the plugin
static gboolean plugin_init(GstPlugin *plugin) {
    // Register the MelSpecFilter type with GStreamer
    if (!gst_element_register(plugin, PLUGIN_NAME, GST_RANK_NONE, GST_TYPE_MEL_SPEC_FILTER)) {
        g_printerr("Failed to register element type %s\n", PLUGIN_NAME);
        return FALSE;
    }
    return TRUE;
}

// Define the plugin structure
GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    mel_spectrogram,
    "Mel Spectrogram Plugin",
    plugin_init,
    "1.0.0",
    "LGPL",
    "GStreamer",
    PACKAGE
)

