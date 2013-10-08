#include "recognizer.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vector>
#include <algorithm>

namespace {

    void fixBox(cv::RotatedRect& box) {
        if (box.angle < -45.0) {
            std::swap(box.size.width, box.size.height);
            box.angle += 90.0f;
        } else
        if (box.angle > 45.0) {
            std::swap(box.size.width, box.size.height);
            box.angle -= 90.0f;
        }
    }

    void findLicensePlate(const std::vector< std::vector<cv::Point> >& contours,
                          const std::vector<cv::Vec4i>& hier,
                          int id,
                          std::vector<int>& plates)
    {
        #define HIER_NEXT(id) hier[id][0]
        #define HIER_CHILD(id) hier[id][2]
        const int MIN_PLATE_AREA = 400;

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
                if (cv::contourArea(contours[id]) < box.size.area() * 0.7) {
                    findLicensePlate(contours, hier, HIER_CHILD(id), plates);
                    continue;
                }

                fixBox(box);
                double whRatio = (double)box.size.width / box.size.height;
                if (!(3.0 < whRatio && whRatio < 10.0))
                {
                    findLicensePlate(contours, hier, HIER_CHILD(id), plates);
                    continue;
                }

                plates.push_back(id);
            }
        }
    }

    void preprocessImage(cv::Mat img, cv::Mat& gray, cv::Mat& binary){
        cv::Mat pyr, timg;
        cv::pyrDown(img, pyr, cv::Size(img.cols / 2, img.rows / 2));
        cv::pyrUp(pyr, timg, img.size());
        cv::cvtColor(timg, gray, CV_BGR2GRAY);
        cv::Canny(gray, binary, 100, 50, 3);
    }

    static const char * goodLetters = "ABCEHIKMOPTX0123456789";

    bool isNotGoodLetter(char ch) {
        return strchr(goodLetters, ch) == NULL;
    }

    void fixValue(std::string& value) {
        value.erase(std::remove_if(value.begin(), value.end(), isNotGoodLetter), value.end());
        while (value.length() > 0 && !isdigit(value[0])) value.erase(value.begin());
        while (value.length() > 0 && !isdigit(value[value.length() - 1])) value.erase(value.end() - 1);
        if (value.length() < 5) value = "";
    }

}

namespace anpr {

    Recognizer::Recognizer() {
        tessApi.Init(NULL, "eng");
        tessApi.SetVariable("tessedit_char_whitelist", goodLetters);
    }

    bool Recognizer::RecognizePlateNumber(cv::Mat image, std::string& value) {
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

            tessApi.SetImage((uchar*)plateFiltered.data, plateFiltered.cols, plateFiltered.rows, plateFiltered.channels(), plateFiltered.step1());
            tessApi.SetRectangle(0, 0, plateFiltered.cols, plateFiltered.rows);
            value += tessApi.GetUTF8Text();
        }

        fixValue(value);
        return value.length() > 0;
    }

}

