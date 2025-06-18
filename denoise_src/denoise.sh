#!/bin/sh
# used for CH dataset
# define variables
INPUT_FILE=$1
OUTPUT_FILE=$2
NOISE_PROFILE=$3  
SAMPLE_RATE=22050 # fit how sample on mel-spectrogram code
# fetch 0~1 sec from input file and use noiseprof to generate noised audio.
sox "$INPUT_FILE" -n trim 0 2.5 noiseprof "$NOISE_PROFILE"

# noisered "$NOISE_PROFILE" 0.25 : we use noised audio to do denoise and "0.25" is the parameter.
# rate "$SAMPLE_RATE" : specify output file 's sample rate to $SAMPLE_RATE.
# --norm=-1 : help us do audio volume enhancement.
#sox --norm=-1 "$INPUT_FILE" "$OUTPUT_FILE" rate "$SAMPLE_RATE" noisered "$NOISE_PROFILE" 0.000000005

#sox  --norm=-1 "$INPUT_FILE" "$OUTPUT_FILE" noisered "$NOISE_PROFILE" 0.05
sox  "$INPUT_FILE" "$OUTPUT_FILE" noisered "$NOISE_PROFILE" 0.05
