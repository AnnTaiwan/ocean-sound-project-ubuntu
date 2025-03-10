#include <pthread.h>
#include <iostream>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/highgui.hpp>
#define RESIZE true
#define IMAGE_SIZE 64
cv::Mat show_image;
// the function that thread will do
void* cv_show(void* arg) {
    printf("Hello from thread %d!\n", *(int*)arg);
        // show this image
    cv::imshow("Image Display", show_image);
    // wait for any key to stop showing
    cv::waitKey(3000); //ms
    cv::destroyWindow("Image Display");
    return NULL;
}
int main(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cout << "Usage Error: " << argv[0] << " <image.png>\n";
        return -1;
    }
    // thread init
    pthread_t thread1;  // 定義兩個線程
    int thread1_id = 1;
    
    // read image
    std::string image_name = argv[1];
    cv::Mat image = cv::imread(image_name);
    
    if(RESIZE)
    {
        std::cout << "Here are going to resize to " << IMAGE_SIZE << " x " << IMAGE_SIZE << std::endl;
        cv::Mat resized_img;
        cv::resize(image, resized_img, cv::Size(IMAGE_SIZE, IMAGE_SIZE));
        image = resized_img;
    }

    if (image.empty()) {
        std::cerr << "Cannot load " << image_name << std::endl;
        return -1;
    }

    std::cout << "Image size: " << image.rows << " x " << image.cols << std::endl;
    std::cout << "Image channels: " << image.channels() << std::endl;

    // print RGB value
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            cv::Vec3b pixel = image.at<cv::Vec3b>(i, j); // 獲取 BGR 值
            int blue = pixel[0];
            int green = pixel[1];
            int red = pixel[2];
            float blue_f = pixel[0]/255.0;
            float green_f = pixel[1]/255.0;
            float red_f = pixel[2]/255.0;
            if(i == 63 && j < 10)
            {
                std::cout << "Pixel (" << i << ", " << j << "): R=" << red
                           << " G=" << green << " B=" << blue << std::endl;
                std::cout << "Pixel (" << i << ", " << j << "): R=" << red_f
                           << " G=" << green_f << " B=" << blue_f << std::endl;
            }
        }
    }
    show_image = image; // prepare image to show
    // loop this several times
    for(int i = 0; i < 1; i++){
        std::cout << "count : " << i+1 << std::endl;
        if (pthread_create(&thread1, NULL, cv_show, (void*)&thread1_id)) {
            fprintf(stderr, "Error creating thread 1\n");
            return -1;
        }
        std::cout << "MIDDLE\n";
        if (pthread_join(thread1, NULL)) {
            fprintf(stderr, "Error joining thread 1\n");
            return -1;
        }
        std::cout << "count " << i+1 << " thread finishes\n";
    }
    std::cout << "All threads finish.\n";
    return 0;
}

