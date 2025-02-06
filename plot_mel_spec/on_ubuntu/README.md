## Plot Mel Spectrogram with 0-255 Values
* Originally, the colormap used values ranging from 0 to 1, based on the [viridis colormap reference values](https://github.com/mrirecon/view/blob/master/src/colormaps.inc).
    * `plot_mel_spec_from_txt_ver2_for_ocean.cpp` use color map which is 0~1.
* Reference [this colormap](https://github.com/stolk/allrgb/blob/master/write_pgm.h).
In `plot_mel_spec_from_txt_ver3_for_ocean.cpp`, if the output needs to be rendered as an image using `cairo`, **dividing the values by 255 (like below code segment)** results in a visually similar image:  
```c
r = viridis[idx][0] / 255;
g = viridis[idx][1] / 255;
b = viridis[idx][2] / 255;
```
  
### `plot_mel_spec_from_txt_out_RGB_for_ocean.cpp`
* only calculate **RGB value (0-255)**.
* revised from `plot_mel_spec_from_txt_ver3_for_ocean.cpp`, so it used color map (0-255), too.
* mel_spec's RGB colors are stored in three 2D arrays: `arr_r`, `arr_g`, `arr_b`. **Each size is (num_frames, num_mels).**

### In 2025/2/6, 
I change the parameters like `N_MELS`, `AUDIO_LEN`... to meet the requirement of new setting in output mel_image which is 65*64.  
* Modify file:
    * `cal_mel_spec_ver4_for_ocean.cpp`
    * `plot_an_audio.bash`
    * `plot_mel_spec_from_txt_ver3_for_ocean.cpp`
