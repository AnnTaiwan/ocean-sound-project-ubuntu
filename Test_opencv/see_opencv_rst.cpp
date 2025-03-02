#include <iostream>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>

int main()
{
    // read image
    std::string image_name = "image.png";
    cv::Mat image = cv::imread(image_name);

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

            std::cout << "Pixel (" << i << ", " << j << "): R=" << red
                      << " G=" << green << " B=" << blue << std::endl;
        }
    }
    // show this image
    cv::imshow("Image Display", image);
    // wait for any key to stop showing
    cv::waitKey(0);
    return 0;
}

