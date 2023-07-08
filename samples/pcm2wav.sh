#!/bin/bash

input_folder="."
output_folder="."

rm *.wav

# Iterate over PCM files in the input folder
for pcm_file in "$input_folder"/*.raw; do
    # Extract the file name without extension
    file_name=$(basename "$pcm_file" .raw)
    
    # Construct the output file path
    output_file="$output_folder/$file_name.wav"
    
    # Convert PCM to WAV using ffmpeg
    # for falcon
    ffmpeg -f s8 -ar 4096 -ac 1 -i "$pcm_file" "$output_file"
done