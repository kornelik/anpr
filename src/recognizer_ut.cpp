#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "recognizer.h"

const char * test_images[] = {
    "IMAG0593.jpg",
    "IMAG0594.jpg",
    "IMAG0595.jpg",
    "IMAG0596.jpg",
    "IMAG0597.jpg",
    "IMAG0598.jpg",
    "IMAG0599.jpg",
    "IMAG0600.jpg",
    "IMAG0604.jpg",
    "IMAG0608.jpg",
    "IMAG0609.jpg",
    "IMAG0610.jpg",
    "IMAG0611.jpg",
    "IMAG0612.jpg",
    "IMAG0613.jpg"
};

int main()
{
    std::string v;
    anpr::Recognizer r;

    int test_images_count = sizeof(test_images) / sizeof(test_images[0]);
    for (int testi = 0; testi < test_images_count; ++testi) {
        cv::Mat img = cv::imread(std::string("tests/") + test_images[testi]);
        if (r.RecognizePlateNumber(img, v)) {
            std::cout << "OK: " << v << std::endl;
        } else {
            std::cout << "NO" << std::endl;
        }
        cv::namedWindow("Test", CV_WINDOW_AUTOSIZE);
        cv::imshow("Test", img);
        cv::waitKey(0);
    }
    return 0;
}

