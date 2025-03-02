# Ocean Sound Project on Ubuntu
Record the source codes used to complete the flow on the hardware.
It can divide into several category: draw mel-spectrogram, denoise(SoX), detection, ...
## Mel Spectrogram Processing and Visualization

This repository contains scripts and tools for processing audio files to generate mel spectrograms. The mel spectrograms are stored as text files and visualized as images for analysis.

## Directory Structure

```
.
├── dataset/          # Directory containing input audio files
├── spec_txt/         # Directory to store generated mel spectrogram text files
├── images/           # Directory to save mel spectrogram images
├── plot_mel_spec/    # Scripts for processing and plotting spectrograms
├── .gitignore        # Git configuration file to exclude unnecessary files
├── Test_opencv       # Dorectory to test some actions by opencv4/opencv2
├── README.md         # This documentation file
└── .git/             # Git version control metadata
```

## Prerequisites

Before running the scripts, make sure you have the following installed:
- **Python** with libraries:
  - `matplotlib`
  - `numpy`
- **SoX** (Sound eXchange) for audio manipulation
- Required compiled programs for processing and plotting:
  - `cal_mel_spec_ver4_for_ocean`  
  - `plot_mel_spec_from_txt_ver2_for_ocean`

## Usage

### 1. Preparing Audio Files
Place your input audio files (e.g., `.wav`) in the `dataset/` directory.

### 2. Generating Mel Spectrogram Text Files
Run the following command to convert an audio file to a mel spectrogram text file:
```bash
./cal_mel_spec_ver4_for_ocean <input_audio_file> <output_txt_folder>
```

For example:
```bash
./cal_mel_spec_ver4_for_ocean dataset/audio.wav spec_txt/
```

### 3. Generating Spectrogram Images
Use the following command to convert a mel spectrogram text file into an image:
```bash
./plot_mel_spec_from_txt_ver2_for_ocean <input_txt_file> <output_image_file>
```

For example:
```bash
./plot_mel_spec_from_txt_ver2_for_ocean spec_txt/audio.txt images/audio_image.png
```

### 4. Automating Batch Processing
The following Bash script processes all audio files in the `dataset/` directory to generate mel spectrograms and save them as images:

```bash
#!/bin/bash

AUDIO_FOLDER="./dataset"
TXT_FOLDER="./spec_txt"
IMG_FOLDER="./images"

for audio_file in "$AUDIO_FOLDER"/*.wav; do
    audio_name=$(basename "$audio_file" .wav)
    ./cal_mel_spec_ver4_for_ocean "$audio_file" "$TXT_FOLDER/"
    ./plot_mel_spec_from_txt_ver2_for_ocean "$TXT_FOLDER/${audio_name}.txt" "$IMG_FOLDER/${audio_name}_image.png"
done
```

### 5. Additional Example Script
To process a single audio file, use this script:
```bash
#!/bin/bash
# Prepare folders for input and output
TXT_FOLDER="../../spec_txt"             # The folder where mel spectrogram txt files will be saved
IMG_FOLDER="../../images"               # The folder where mel spectrogram images will be saved

if [ $# -lt 1 ]; then
    echo "Usage: $0 <audio.wav>" >&2
    exit 1
fi

# Generate mel spectrogram txt files
./cal_mel_spec_ver4_for_ocean $1 $TXT_FOLDER/
   
# Convert mel spectrogram txt files to images
for txt_file in "$TXT_FOLDER"/*.txt; do
    txt_filename=$(basename "$txt_file")                  # Get the base name of the txt file
    base_name="${txt_filename%.*}"                        # Remove the .txt extension
    image_name="${base_name}_image.png"                   # Add the _image.png suffix
    ./plot_mel_spec_from_txt_ver2_for_ocean "$txt_file" "$IMG_FOLDER/$image_name"
done

echo "Processing completed. Spectrograms saved in $IMG_FOLDER."
```

### Output
- **Mel spectrogram text files** are stored in `spec_txt/`.
- **Mel spectrogram images** are stored in `images/`.

## Examples

1. **Input Audio File:**
   - `dataset/audio.wav`

2. **Generated Files:**
   - Mel spectrogram text: `spec_txt/audio.txt`
   - Spectrogram image: `images/audio_image.png`





