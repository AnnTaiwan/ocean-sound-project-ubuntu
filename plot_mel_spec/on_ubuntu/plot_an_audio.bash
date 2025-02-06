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
    ./plot_mel_spec_from_txt_ver3_for_ocean "$txt_file" "$IMG_FOLDER/$image_name"
done

echo "Processing completed. Spectrograms saved in $IMG_FOLDER."


