#include <opencv2/core/core.hpp>
#include <string>

namespace anpr {

class Recognizer {
private:
    Recognizer(const Recognizer&) {};

    class Impl;
    Impl* impl_;

public:
    Recognizer();
    ~Recognizer();
    bool RecognizePlateNumber(cv::Mat image, std::string& value);
};

}
