#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/highgui.hpp>

#define RESIZE true
#define IMAGE_SIZE 600

cv::Mat show_image;
std::mutex image_mutex;
bool keep_running = true;

// Display Thread: Continuously updates the GUI
void display_thread() {
    std::cout << "\33[36mHello from dsplay thread, press ESC to stop showing image and quit the program.\33[0m\n";
    cv::namedWindow("Threaded Display", cv::WINDOW_AUTOSIZE);

    while (keep_running) {
        {
            std::lock_guard<std::mutex> lock(image_mutex);
            if (!show_image.empty()) {
                cv::imshow("Threaded Display", show_image);
            }
        }

        int key = cv::waitKey(3000);
        if (key == 27) {  // Press ESC to exit
            keep_running = false;
        }
    }

    cv::destroyAllWindows();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <image.png>\n";
        return -1;
    }

    // Read image
    std::string image_name = argv[1];
    cv::Mat image = cv::imread(image_name);
    
    if (image.empty()) {
        std::cerr << "Cannot load " << image_name << std::endl;
        return -1;
    }

    // Resize image if needed
    if (RESIZE) {
        cv::resize(image, image, cv::Size(IMAGE_SIZE, IMAGE_SIZE));
    }

    // Start display thread
    std::thread gui_thread(display_thread);
    // Main Thread: Perform computation while the display thread runs
    for (int i = 0; i < 5; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(3));  // Simulate computation

        cv::Mat processed_image = image;
        // cv::GaussianBlur(image, processed_image, cv::Size(5, 5), 0);  // Apply Gaussian blur
        cv::flip(processed_image, processed_image, 1);

        // Update the image safely
        {
            std::lock_guard<std::mutex> lock(image_mutex); // When lock_guard goes out of scope, it automatically calls unlock().
            show_image = processed_image.clone();
        }

        std::cout << "Updated image at iteration " << i + 1 << std::endl;
    }

    // Wait for the GUI thread to finish
    gui_thread.join();
    std::cout << "\33[36mProgram successfully finishes all threads.\n\33[0m";
    return 0;
}

