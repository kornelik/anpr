#include "recognizer.h"

#include "ocr_char.h"
#include "util.h"

#include <tesseract/baseapi.h>

#include <iostream>
#include <vector>
#include <algorithm>

namespace anpr {
    const double RIGHT_ANGLE = 90.0f;
    const double HALF_RIGHT_ANGLE = RIGHT_ANGLE / 2.;
    const double BLUR_SIZE = 3;
    const double CANNY_PARAM = 100;
    const size_t MAX_VALUE_LENGTH = 5;
    const double PLATE_HEIGHT_RATIO = 0.6f;
    const double PLATE_WIDTH_BOUND = 10;
    const double POSSIBLE_RECT_SIZE = 7;
	const size_t CONTOUR_THICKNESS = 2;
	const size_t LINE_CONNECTIVITY = 8;
	const size_t MAX_COLOR_COMP = 255;
	const size_t MAX_THRESHOLD = 255;
	const size_t THRESHOLD_CONST = 3;

    class Recognizer::Impl {
    private:
        OCRChar ocrLetter_, ocrNumber_;

        static void fixBox(cv::RotatedRect& box) {
			if (box.angle < -HALF_RIGHT_ANGLE) {
                std::swap(box.size.width, box.size.height);
				box.angle += RIGHT_ANGLE;
            } else
				if (box.angle > HALF_RIGHT_ANGLE) {
                    std::swap(box.size.width, box.size.height);
					box.angle -= RIGHT_ANGLE;
                }
        }

        static void findLicensePlate(const std::vector< std::vector<cv::Point> >& contours,
                const std::vector<cv::Vec4i>& hier,
                int id,
                std::vector<int>& plates)
        {
            #define HIER_NEXT(id) hier[id][0]
            #define HIER_CHILD(id) hier[id][2]
			const int CHILDREN_COUNT = 0;
			const int MAX_CHILDREN_COUNT = 7;
            const int MIN_PLATE_AREA = 400;
            const double AREA_SHAPE_EPS = 0.7;
            const double MIN_BOX_RATIO = 3.0;
            const double MAX_BOX_RATIO = 10.0;

            for(;id != -1; id = HIER_NEXT(id)) {
                if (HIER_CHILD(id) == -1) continue;

                if (cv::contourArea(contours[id]) > MIN_PLATE_AREA) {
					int childCnt = CHILDREN_COUNT;
                    for (int child = HIER_CHILD(id); child != -1; child = HIER_NEXT(child)) ++childCnt;
					if (childCnt < MAX_CHILDREN_COUNT) {
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
            #undef HIER_NEXT
            #undef HIER_CHILD
        }

        static void preprocessImage(cv::Mat img, cv::Mat& gray, cv::Mat& binary){
            cv::Mat timg;
            cv::cvtColor(img, gray, CV_BGR2GRAY);
			cv::blur(gray, timg, cv::Size(BLUR_SIZE, BLUR_SIZE));
			cv::Canny(timg, binary, CANNY_PARAM, CANNY_PARAM / 2, CANNY_PARAM / 20);
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
			if (value.length() < MAX_VALUE_LENGTH) value = "";
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

            cv::adaptiveThreshold(plate, plate2, MAX_THRESHOLD, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY,
		plate.cols + 1 - (plate.cols & 1), THRESHOLD_CONST);
			cv::Canny(plate2, pcanny, CANNY_PARAM, CANNY_PARAM - 30, CANNY_PARAM / 20);

            cv::findContours(pcanny, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
            std::sort(contours.begin(), contours.end(), contByArea);
            std::vector<cv::Rect> possible;
            for (size_t i = 0; i < contours.size(); ++i) {
                cv::Rect rect = cv::boundingRect(contours[i]);
				if (rect.height < PLATE_HEIGHT_RATIO * plate.size().height || rect.width <= 2 ||
					rect.x < plate.size().width / PLATE_WIDTH_BOUND)  continue;

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
            }
			if (possible.size() == POSSIBLE_RECT_SIZE) {
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
                cv::drawContours(image, contours, plates[i], cv::Scalar(0, 0, MAX_COLOR_COMP),
					CONTOUR_THICKNESS, LINE_CONNECTIVITY, hierarchy, 0, cv::Point());

                cv::Rect plateRect  = cv::boundingRect(contours[plates[i]]);
                cv::Mat plateImage  = cv::Mat(gray, plateRect), plateStraight;
                cv::RotatedRect box = cv::minAreaRect(contours[plates[i]]);
				fixBox(box);

                box.center.x -= plateRect.x;
                box.center.y -= plateRect.y;

				/* Define preferred points */
				cv::Point2f rectpoints[4];
				rectpoints[1] = cv::Point2f(0, 0);
				rectpoints[2] = cv::Point2f(box.size.width, 0);
				rectpoints[3] = cv::Point2f(box.size.width, box.size.height);
				rectpoints[0] = cv::Point2f(0, box.size.height);
				
				/* Get rotated rect points */
				cv::Point2f boxPoints[4];
				box.points(boxPoints);

				/* Get transformation matrix and perform transformation */
				cv::Mat transform = cv::getPerspectiveTransform(boxPoints, rectpoints);
				cv::warpPerspective(plateImage, plateStraight, transform, cv::Size(box.size.width, box.size.height));

				/* Parse the plate number */
				value = parsePlate(plateStraight);

				if (value.length() == 7) {
					return true;
				}
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

#undef SYMBOLS
#undef SYMBOLS_NUMBERS
#undef SYMBOLS_CHARS
