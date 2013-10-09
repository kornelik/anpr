#include "recognizer.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

#include <tesseract/baseapi.h>

#include <iostream>
#include <vector>
#include <algorithm>

namespace anpr {

    void debug(const cv::Mat& image) {
        cv::namedWindow("Debug", 0);
        cv::imshow("Debug", image);
        cv::waitKey(0);
    }
    void debug(const cv::Mat& i1, const cv::Mat& i2,const cv::Mat& i3) {
        cv::namedWindow("Debug", 0);

        cv::Mat image(cv::Size(i1.size().width + i2.size().width + i3.size().width, std::max(std::max(i1.size().height, i2.size().height), i3.size().height)), i1.type());
        i1.copyTo( image(cv::Rect(cv::Point(0, 0), i1.size())) );
        i2.copyTo( image(cv::Rect(cv::Point(i1.size().width, 0), i2.size())) );
        i3.copyTo( image(cv::Rect(cv::Point(i1.size().width + i2.size().width, 0), i2.size())) );

        cv::imshow("Debug", image);
        cv::waitKey(0);
    }

    class OCRChar {
    private:
        cv::KNearest oracle;

    public:
        static void safeBinaryResize(const cv::Mat& from, cv::Mat& out, cv::Size size) {
            float factor = std::min((float)size.width / from.size().width, (float)size.height / from.size().height);
            cv::Mat sized;
            cv::resize(from, sized, cv::Size(from.size().width * factor, from.size().height * factor), 0, 0, cv::INTER_CUBIC);
            cv::Mat dst(size, CV_8U);
            dst.setTo(cv::Scalar(255, 255, 255));
            sized.copyTo(dst(cv::Rect(cv::Point((size.width - sized.size().width) / 2, (size.height - sized.size().height) / 2), sized.size())));
            cv::threshold(dst, out, 100, 255, cv::THRESH_BINARY);
        }

        OCRChar(const std::string& learnpath, const std::string& allchars)
        {
            cv::Mat trainData(allchars.length() * 4, 20 * 32, CV_32FC1);
            cv::Mat trainClasses(allchars.length() * 4, 1, CV_32FC1);
            size_t counter = 0;
            for (size_t i = 0; i < allchars.size(); ++i) {
                {
                    std::string path = learnpath + allchars[i] + ".png";
                    cv::Mat inp = cv::imread(path, 0), out;
                    safeBinaryResize(inp, out, cv::Size(20, 32));
                    for (size_t j = 0; j < 20 * 32; ++j) {
                        ((float*)trainData.data)[counter++] = out.data[j];
                    }
                    for (size_t j = 0; j < 20 * 32; ++j) {
                        ((float*)trainData.data)[counter++] = inp.data[j];
                    }
                }
                {
                    std::string path = learnpath + allchars[i] + "2.png";
                    cv::Mat inp = cv::imread(path, 0), out;
                    safeBinaryResize(inp, out, cv::Size(20, 32));
                    for (size_t j = 0; j < 20 * 32; ++j) {
                        ((float*)trainData.data)[counter++] = out.data[j];
                    }
                    for (size_t j = 0; j < 20 * 32; ++j) {
                        ((float*)trainData.data)[counter++] = inp.data[j];
                    }
                }
                ((float*)trainClasses.data)[i * 4] = allchars[i];
                ((float*)trainClasses.data)[i * 4 + 1] = allchars[i];
                ((float*)trainClasses.data)[i * 4 + 2] = allchars[i];
                ((float*)trainClasses.data)[i * 4 + 3] = allchars[i];
            }
            oracle.train(trainData, trainClasses);
        }

        char Get(cv::Mat image) {
            cv::Mat bin;
            safeBinaryResize(image, bin, cv::Size(20, 32));
            cv::Mat sample(1, 32 * 20, CV_32FC1);
            for (size_t i = 0; i < 32 * 20; ++i) ((float*)sample.data)[i] = bin.data[i];
            return (char)(oracle.find_nearest(sample, 1) + 1e-5);
        }
    };

    class Recognizer::Impl {
    private:
        tesseract::TessBaseAPI ocr_;
        OCRChar ocrLetter_, ocrNumber_;

        static void fixBox(cv::RotatedRect& box) {
            if (box.angle < -45.0) {
                std::swap(box.size.width, box.size.height);
                box.angle += 90.0f;
            } else
                if (box.angle > 45.0) {
                    std::swap(box.size.width, box.size.height);
                    box.angle -= 90.0f;
                }
        }

        static void findLicensePlate(const std::vector< std::vector<cv::Point> >& contours,
                const std::vector<cv::Vec4i>& hier,
                int id,
                std::vector<int>& plates)
        {
            #define HIER_NEXT(id) hier[id][0]
            #define HIER_CHILD(id) hier[id][2]
            const int MIN_PLATE_AREA = 400;
            const double AREA_SHAPE_EPS = 0.7;
            const double MIN_BOX_RATIO = 3.0;
            const double MAX_BOX_RATIO = 10.0;

            for(;id != -1; id = HIER_NEXT(id)) {
                if (HIER_CHILD(id) == -1) continue;

                if (cv::contourArea(contours[id]) > MIN_PLATE_AREA) {
                    int childCnt = 0;
                    for (int child = HIER_CHILD(id); child != -1; child = HIER_NEXT(child)) ++childCnt;
                    if (childCnt < 7) {
                        findLicensePlate(contours, hier, HIER_CHILD(id), plates);
                        continue;
                    }

                    cv::RotatedRect box = cv::minAreaRect(contours[id]);
                    if (cv::contourArea(contours[id]) < box.size.area() * AREA_SHAPE_EPS) {
                        findLicensePlate(contours, hier, HIER_CHILD(id), plates);
                        continue;
                    }

                    fixBox(box);
                    double whRatio = (double)box.size.width / box.size.height;
                    if (whRatio < MIN_BOX_RATIO || whRatio > MAX_BOX_RATIO)
                    {
                        findLicensePlate(contours, hier, HIER_CHILD(id), plates);
                        continue;
                    }

                    plates.push_back(id);
                }
            }
        }

        static void preprocessImage(cv::Mat img, cv::Mat& gray, cv::Mat& binary){
            cv::Mat pyr, timg;
            cv::pyrDown(img, pyr, cv::Size(img.cols / 2, img.rows / 2));
            cv::pyrUp(pyr, timg, img.size());
            cv::cvtColor(timg, gray, CV_BGR2GRAY);
            cv::Canny(gray, binary, 100, 50, 3);
        }

        #define SYMBOLS "ABCEHIKMOPTX0123456789"
        #define SYMBOLS_NUMBERS "0123456789"
        #define SYMBOLS_CHARS "ABCEHIKMOPTX"

        static bool isNotGoodLetter(char ch) {
            return strchr(SYMBOLS, ch) == NULL;
        }

        static void fixValue(std::string& value) {
            value.erase(std::remove_if(value.begin(), value.end(), isNotGoodLetter), value.end());
            while (value.length() > 0 && !isdigit(value[0])) value.erase(value.begin());
            while (value.length() > 0 && !isdigit(value[value.length() - 1])) value.erase(value.end() - 1);
            if (value.length() < 5) value = "";
        }

        static bool rectContain(cv::Rect r1, cv::Rect r2) {
            return r1.x <= r2.x && r1.y <= r2.y && r1.x + r1.width >= r2.x + r2.width && r1.y + r1.height >= r2.y + r2.height;
        }

        static bool rectByX(const cv::Rect& r1, const cv::Rect& r2) {
            return r1.x < r2.x;
        }

        std::string parsePlate(const cv::Mat& plate) {
            cv::Mat psize, pcanny;

            std::vector< std::vector<cv::Point> > contours;
            cv::Mat paint(plate.size(), CV_8U);
            cv::Canny(plate, pcanny, 100, 50, 3);
            plate.copyTo(paint);
            cv::findContours(pcanny, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

            std::vector<cv::Rect> possible;
            for (size_t i = 0; i < contours.size(); ++i) {
                cv::Rect rect = cv::boundingRect(contours[i]);
                if (rect.height < 0.6 * plate.size().height || rect.width <= 5)  continue;

                bool add = true;
                for (size_t j = 0; j < possible.size(); ++j) {
                    if (rectContain(possible[j], rect)) {
                        possible[j] = rect;
                        add = false;
                        break;
                    }
                    if (rectContain(rect, possible[j])) {
                        add = false;
                        break;
                    }
                }
                if (add) {
                    possible.push_back(rect);
                }
            }
            if (possible.size() == 7) {
                std::string result;
                std::sort(possible.begin(), possible.end(), rectByX);

                cv::Mat plate2;
                cv::adaptiveThreshold(plate, plate2, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 0);

                for (size_t i = 0; i < 4; ++i) result += ocrNumber_.Get(cv::Mat(plate2, possible[i]));
                for (size_t i = 4; i < 6; ++i) result += ocrLetter_.Get(cv::Mat(plate2, possible[i]));
                for (size_t i = 6; i < 7; ++i) result += ocrNumber_.Get(cv::Mat(plate2, possible[i]));

                fixValue(result);
                return result;
            }

            ocr_.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
            ocr_.SetVariable("tessedit_char_whitelist", SYMBOLS);
            ocr_.SetImage((uchar*)plate.data, plate.cols, plate.rows, plate.channels(), plate.step1());
            ocr_.SetRectangle(0, 0, plate.cols, plate.rows);

            std::string result = ocr_.GetUTF8Text();
            fixValue(result);
            return result;
        }

    public:
        Impl() : ocrLetter_("samples/", SYMBOLS_CHARS), ocrNumber_("samples/", SYMBOLS_NUMBERS) {
            ocr_.Init(NULL, "eng");
        }

        bool RecognizePlateNumber(cv::Mat image, std::string& value) {
            std::vector< std::vector<cv::Point> > contours;
            std::vector<cv::Vec4i> hierarchy;
            std::vector<int> plates;

            cv::Mat gray, binary;
            preprocessImage(image, gray, binary);
            cv::findContours(binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
            if (contours.size() > 0) {
                findLicensePlate(contours, hierarchy, 0, plates);
            }
            if (plates.size() == 0) return false;

            value = "";
            for (size_t i = 0; i < plates.size(); ++i) {
                cv::drawContours(image, contours, plates[i], cv::Scalar(0, 0, 255), 2, 8, hierarchy, 0, cv::Point());

                cv::Rect plateRect  = cv::boundingRect(contours[plates[i]]);
                cv::Mat plateImage  = cv::Mat(gray, plateRect), plateStraight, plateFiltered;
                cv::RotatedRect box = cv::minAreaRect(contours[plates[i]]);
                fixBox(box);

                cv::Mat rotation  = cv::getRotationMatrix2D(cv::Point(box.center.x - plateRect.x, box.center.y - plateRect.y), box.angle, 1);
                cv::warpAffine(plateImage, plateStraight, rotation, plateImage.size(), cv::INTER_CUBIC);
                cv::getRectSubPix(plateStraight, box.size, cv::Point(box.center.x - plateRect.x, box.center.y - plateRect.y), plateFiltered);

                value = parsePlate(plateFiltered);
                if (value.length()) return true;
            }

            return false;
        }
    };

    Recognizer::Recognizer() : impl_(new Impl()) { }

    Recognizer::~Recognizer() { delete impl_; }

    bool Recognizer::RecognizePlateNumber(cv::Mat image, std::string& value) {
        return impl_->RecognizePlateNumber(image, value);
    }
}

