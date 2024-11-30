## Create a custom gstreamer plugin
### plot mel spectrogram
#### 1. how to get the plugin.so file
    `$ sh compile_gst_plugin.sh`
#### 2. how to copy this so file into my testing plugin library: `/home/ann/my-gstreamer-plugins/`
* create this directory to store my plugin
```bash
export GST_PLUGIN_PATH=~/my-gstreamer-plugins:$GST_PLUGIN_PATH
```
* copy this file into above dir
```bash
cp mel_spectrogram.so ~/my-gstreamer-plugins/
```
* check if it sucessfully registers
```bash
gst-inspect-1.0 --plugin | grep mel # see the plugin library
gst-inspect-1.0 mel_spectrogram     # see the plugin detail
```
#### 3. use this plugin to test its function
>still exists some errors, but it can really be used.  
>So, it will be corrected later.  
* testing command(my plugin is called **mel_spectrogram**  
```bash
gst-launch-1.0 filesrc location=audio_for_test/dolphin_M1_20201015_202014.wav ! decodebin ! audioconvert ! audioresample ! audio/x-raw,format=F32LE,channels=1,rate=48000 ! mel_spectrogram ! fakesink
```
