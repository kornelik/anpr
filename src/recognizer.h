#include <opencv2/core/core.hpp>
#include <string>
#include <tesseract/baseapi.h>

namespace anpr {

class Recognizer {
    private:
        tesseract::TessBaseAPI tessApi;

    public:
        Recognizer();

        bool RecognizePlateNumber(cv::Mat image, std::string& value);
};

}
