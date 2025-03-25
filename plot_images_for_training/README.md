# Audio Spectrogram Generator

## 📌 Overview
This shell script processes a folder containing audio files (`.wav`) and generates spectrogram images using **GStreamer**. The generated images are renamed to match the corresponding audio filenames and stored in a specified label folder.

## 📂 Directory Structure
```
plot_images_for_training(root)/
│── dataset_audio/            # Audio dataset directory
│   ├── fish_whale_add_noise20db_CrowdTalking/
│   │   ├── audio1.wav
│   │   ├── audio2.wav
│   │   └── ...
│
│── dataset_images/           # Image dataset directory
│   ├── mel_images/           # Temporary folder for spectrogram images
│   ├── fish_whale_add_noise20db_CrowdTalking_imgs/  # Final labeled images
│
│── src/
│   ├── plot_dataset.sh       # Shell script for processing audios
│   └── README.md             # Documentation
```

## 🚀 How to Use

### **1️⃣ Run the script**
Execute the script inside the `src/` directory:
```bash
./plot_dataset.sh <AUDIO_FOLDER> <IMG_FOLDER> <LABEL_FOLDER>
```
- `<AUDIO_FOLDER>`: Path to the folder containing `.wav` audio files.
- `<IMG_FOLDER>`: Path to store spectrogram images (`mel_images/` folder required).
- `<LABEL_FOLDER>`: Path to store the final categorized images.

### **2️⃣ Example Command**
```bash
./plot_dataset.sh ../dataset_audio/fish_whale_add_noise20db_CrowdTalking/ \
                  ../dataset_images/mel_images/ \
                  ../dataset_images/fish_whale_add_noise20db_CrowdTalking_imgs/
```

## ⚙️ Script Details

- **Checks for required arguments** and prints a usage message if incorrect.
- **Creates necessary directories** if they don’t exist.
- **Processes each `.wav` file**:
  1. Extracts the filename.
  2. Runs the **GStreamer** pipeline to generate spectrogram images.
  3. Moves and renames the generated images from `image_X.png` to `{audio_filename}_X.png` inside the label folder.

## 📌 Dependencies
* Ensure **GStreamer** is installed.  
* Ensure `GStreamer plugin(.so)` is at the `my-gstreamer-plugins` directory.
* The `spectrogram` plug used in shell file is the same as the plugin made in`../FINAL_gst-spectrogram_new_plot`. Just remove some output info to let the view cleaner, and the revised version is saved in `FINAL_gst-spectrogram_new_plot_src`.
## 📞 Contact
For issues or improvements, please open an issue on GitHub.

---

**Author:** Ann 
📅 Last Updated: `25 March 2025`
```
