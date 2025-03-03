#include <iostream>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/highgui.hpp>
#define RESIZE true
#define IMAGE_SIZE 64
int main(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cout << "Usage Error: " << argv[0] << " <image.png>\n";
        return -1;
    }
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
    // show this image
    cv::imshow("Image Display", image);
    // wait for any key to stop showing
    cv::waitKey(0);
    return 0;
}

