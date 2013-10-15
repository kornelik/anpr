#include <opencv2/opencv.hpp>
#include <string>

namespace anpr {

class OCRChar {
private:
    OCRChar(const OCRChar&) {};

    class Impl;
    Impl* impl_;

public:
    OCRChar(const std::string& learn_path, const std::string& possible_chars);
    virtual ~OCRChar();

    char Classify(cv::Mat image);
};

}
