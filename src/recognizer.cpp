#include "recognizer.h"

#include "ocr_char.h"
#include "util.h"

#include <tesseract/baseapi.h>

#include <iostream>
#include <vector>
#include <algorithm>

namespace anpr {

    class Recognizer::Impl {
    private:
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
            cv::Mat timg;
            cv::cvtColor(img, gray, CV_BGR2GRAY);
            cv::blur(gray, timg, cv::Size(3, 3));
            cv::Canny(timg, binary, 100, 50, 5);
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

        static bool isRectContains(const cv::Rect& r1, const cv::Rect& r2) {
            return r1.x <= r2.x && r1.y <= r2.y && r1.x + r1.width >= r2.x + r2.width && r1.y + r1.height >= r2.y + r2.height;
        }

        static bool isPointInRect(const cv::Rect& r, const cv::Point& p) {
            return p.x >= r.x && p.x < r.x + r.width && p.y >= r.y && p.y < r.y + r.height;
        }

        static bool isRectIntersectsGood(cv::Rect rct1, cv::Rect rct2) {
            cv::Rect r1(rct1.x + 2, rct1.y + 2, rct1.width - 4, rct1.height - 4);
            cv::Rect r2(rct2.x + 2, rct2.y + 2, rct2.width - 4, rct2.height - 4);
            return isPointInRect(r1, r2.br()) ||
                   isPointInRect(r1, r2.tl()) ||
                   isPointInRect(r1, cv::Point(r2.x, r2.y + r2.height - 1)) ||
                   isPointInRect(r1, cv::Point(r2.x + r2.width - 1, r2.y)) ||
                   isPointInRect(r2, r1.br()) ||
                   isPointInRect(r2, r1.tl()) ||
                   isPointInRect(r2, cv::Point(r1.x, r1.y + r1.height - 1)) ||
                   isPointInRect(r2, cv::Point(r1.x + r1.width - 1, r1.y));
        }

        static cv::Rect rectUnion(cv::Rect r1, cv::Rect r2) {
            int minx = std::min(r1.x, r2.x);
            int miny = std::min(r1.y, r2.y);
            int maxx = std::max(r1.x + r1.width, r2.x + r2.width);
            int maxy = std::max(r1.y + r1.height, r2.y + r2.height);
            return cv::Rect(minx, miny, maxx - minx, maxy - miny);
        }

        static bool rectByX(const cv::Rect& r1, const cv::Rect& r2) {
            return r1.x < r2.x;
        }

        static bool contByArea(const cv::vector<cv::Point>& r1, const cv::vector<cv::Point>& r2) {
            return cv::contourArea(r1) < cv::contourArea(r2);
        }

        std::string parsePlate(const cv::Mat& plate) {
            cv::Mat psize, pcanny;

            std::vector< std::vector<cv::Point> > contours;
            cv::Mat paint(plate.size(), CV_8U), plate2;

            cv::adaptiveThreshold(plate, plate2, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, plate.cols + 1 - (plate.cols & 1),  3);
            cv::Canny(plate2, pcanny, 100, 70, 5);

            //debug(plate);
            //debug(plate2);
            //debug(pcanny);

            cv::findContours(pcanny, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
            std::sort(contours.begin(), contours.end(), contByArea);
            std::vector<cv::Rect> possible;
            for (size_t i = 0; i < contours.size(); ++i) {
                cv::Rect rect = cv::boundingRect(contours[i]);
                if (rect.height < 0.6 * plate.size().height || rect.width <= 2 || rect.x < plate.size().width / 10)  continue;

                /*plate.copyTo(paint);
                for (size_t j = 0; j < possible.size(); ++j) {
                    cv::rectangle(paint, possible[j], cv::Scalar(0, 0, 0));
                }
                cv::rectangle(paint, rect, cv::Scalar(50, 50, 50));
                debug(paint);
                */

                bool add = true;
                for (size_t j = 0; j < possible.size(); ++j) {
                    if (isRectContains(rect, possible[j])) {
                        add = false;
                        break;
                    }
                }
                if (add)
                for (size_t j = 0; j < possible.size(); ++j) {
                    if (isRectContains(possible[j], rect)) {
                        possible[j] = rect;
                        add = false;
                        break;
                    }
                }
                if (add)
                for (size_t j = 0; j < possible.size(); ++j) {
                    if (isRectIntersectsGood(rect, possible[j])) {
                        possible[j] = rectUnion(rect, possible[j]);
                        add = false;
                        break;
                    }
                }
                if (add) {
                    possible.push_back(rect);
                } else continue;

                /*plate.copyTo(paint);
                for (size_t j = 0; j < possible.size(); ++j) {
                    cv::rectangle(paint, possible[j], cv::Scalar(0, 0, 0));
                }
                debug(paint);*/
            }
            if (possible.size() == 7) {
                std::string result;
                std::sort(possible.begin(), possible.end(), rectByX);

                for (size_t i = 0; i < 4; ++i) result += ocrNumber_.Classify(cv::Mat(plate, possible[i]));
                for (size_t i = 4; i < 6; ++i) result += ocrLetter_.Classify(cv::Mat(plate, possible[i]));
                for (size_t i = 6; i < 7; ++i) result += ocrNumber_.Classify(cv::Mat(plate, possible[i]));

                fixValue(result);
                return result;
            }

            cv::Mat pl = cv::Mat(plate, cv::Rect(plate.cols / 6, 1, plate.cols - plate.cols / 6 - 1, plate.rows - 1));
            ocr_.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
            ocr_.SetVariable("tessedit_char_whitelist", SYMBOLS);
            ocr_.SetImage((uchar*)pl.data, pl.cols, pl.rows, pl.channels(), pl.step1());
            ocr_.SetRectangle(0, 0, pl.cols, pl.rows);
            std::string result = ocr_.GetUTF8Text();
            fixValue(result);
            return result;
        }

        tesseract::TessBaseAPI ocr_;

    public:
        Impl(const std::string& learn_path) : ocrLetter_(learn_path, SYMBOLS_CHARS), ocrNumber_(learn_path, SYMBOLS_NUMBERS) {
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

                box.center.x -= plateRect.x;
                box.center.y -= plateRect.y;

                cv::Mat rotation  = cv::getRotationMatrix2D(cv::Point(box.center.x, box.center.y), box.angle, 1);
                /*cv::Point2f src[4];
                cv::Point2f dst[4];
                box.points(src);
                dst[1] = cv::Point2f(0, 0);
                dst[2] = cv::Point2f(plateImage.cols, 0);
                dst[0] = cv::Point2f(0, plateImage.rows);
                cv::Mat rotation = cv::getAffineTransform(src, dst); */
                cv::warpAffine(plateImage, plateStraight, rotation, plateImage.size(), cv::INTER_CUBIC);
                cv::getRectSubPix(plateStraight, box.size, cv::Point(box.center.x, box.center.y), plateFiltered);

                value = parsePlate(plateFiltered);
                if (value.length()) return true;
            }

            return false;
        }
    };

    Recognizer::Recognizer(const std::string& learn_path) : impl_(new Impl(learn_path)) { }

    Recognizer::~Recognizer() { delete impl_; }

    bool Recognizer::RecognizePlateNumber(cv::Mat image, std::string& value) {
        return impl_->RecognizePlateNumber(image, value);
    }
}

