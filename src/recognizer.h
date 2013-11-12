#pragma once

#include <opencv2/opencv.hpp>
#include <string>

/*
Using pointer to implementation idiom. It allows fast rebuilding of project and easy testing
*/

namespace anpr {

class Recognizer {
private:
    Recognizer(const Recognizer&) {};

    class Impl;
    Impl* impl_;

public:
    // learn_path - path to directory with test sample
    Recognizer(const std::string& learn_path);
    ~Recognizer();
    // Method to recognize plate number.
    // Returns if number was recognized
    // image - object, that incaplulates image
    // value - object, where result will be stored
    bool RecognizePlateNumber(cv::Mat image, std::string& value);
};

}
