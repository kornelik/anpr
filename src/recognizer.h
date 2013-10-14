#include <opencv2/opencv.hpp>
#include <string>

namespace anpr {
	
class Recognizer {
private:
    Recognizer(const Recognizer&) {};

    class Impl;
    Impl* impl_;

public:
    Recognizer(const std::string& learn_path);
    ~Recognizer();
    bool RecognizePlateNumber(cv::Mat image, std::string& value);
};

}
