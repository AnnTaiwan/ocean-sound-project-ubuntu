#!/bin/bash

# Set source and destination directories
SOURCE_DIR=$1 # audio folder
DEST_DIR=$2 # image folder need end with mel_images/ folder
CATEGORY_DIR=$3 # label folder, final destnation
echo "It's going to plot the audios in one folder into a bunch of mel_spec.img(s)"
if [ "$#" -lt 3 ]; then
    echo "Please follow this format: sh plopt_dataset.sh <AUDIO_FOLDER_name> <IMG_FOLDER_name> <Label_folder>"
    exit 1
fi
# Ensure the destination directory exists
mkdir -p "$DEST_DIR"
mkdir -p "$CATEGORY_DIR"

# Loop through all .wav files in the source directory
for audio_file in "$SOURCE_DIR"/*.wav; do
    # Extract the base filename (without path and extension)
    base_name=$(basename "$audio_file" .wav)

    # Define the default output image naming pattern (image_1.png, image_2.png, ...)
    echo $base_name "is processing..."
    # Run the GStreamer pipeline to generate spectrogram images
    gst-launch-1.0 filesrc location="$audio_file" ! decodebin ! audioconvert ! audioresample ! \
        audio/x-raw,format=F32LE,channels=1,rate=22050 ! spectrogram ! fakesink 
    # Rename output images to match the audio filename
    # the default output image naming pattern: (image_1.png, image_2.png, ...)
    for img in "$DEST_DIR"/image_*.png; do
        img_num=$(echo "$img" | grep -oE '[0-9]+')  # Extract the image number
        mv "$img" "$CATEGORY_DIR/${base_name}_${img_num}.png"  # Rename to {audio_filename}_X.png
    done
done

echo "All audio files have been processed!"

