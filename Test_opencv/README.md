# Multithreaded Image Display with OpenCV

## Overview

This program demonstrates how to use multithreading with OpenCV to display and process images concurrently using C++. The main thread loads and processes an image, while a separate thread continuously updates the display. A mutex is used to ensure safe access to the shared image resource.

## Features
- Loads an image from a file.
- Resizes the image to a fixed size (optional, controlled by `#define RESIZE`).
- Starts a separate thread to display the image.
- The main thread applies transformations (flipping the image) and updates the displayed image safely using a mutex.
- Press the `ESC` key to exit the program.

## Dependencies

Ensure you have the following installed:
- OpenCV (tested with OpenCV 4)
- A C++ compiler supporting C++11 or later (e.g., g++, clang++)

## Compilation & Usage

### 1. Compile the program:
```sh
 g++ -std=c++11 cv_thread_mutex.cpp -o cv_thread_mutex `pkg-config --cflags --libs opencv4` -lpthread
```
or
```sh
make clean all
```
### 2. Run the program with an image:
```sh
 ./cv_thread_mutex <image_path>
```
Example:
```sh
 ./cv_thread_mutex sample.png
```

## Code Explanation

- **Image Loading & Resizing:**
  - The main thread reads an image and resizes it if `RESIZE` is set to `true`.

- **Threaded Display:**
  - A separate `display_thread` continuously displays the image using `cv::imshow()`.
  - Press `ESC` to exit the display loop and terminate the program.

- **Main Thread Processing:**
  - The main thread simulates processing by flipping the image every 3 seconds and updating the displayed image safely using a mutex.
  
- **Thread Synchronization:**
  - A `std::mutex` ensures thread-safe access when updating the displayed image.

## Notes
- Ensure OpenCV is correctly installed and linked.
- If `cv::imshow()` doesn't update properly, reduce the `cv::waitKey()` delay in `display_thread()`.



