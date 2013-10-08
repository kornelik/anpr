#include "recognizer.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vector>
#include <cstdio>
#include <algorithm>

using std::vector;

namespace {

    void findLicensePlate(  const vector< vector<cv::Point> >& contours,
                            const vector<cv::Vec4i>& hier,
                            int id,
                            vector<int>& plates)
    {
        for(;id != -1; id = hier[id][0]) {
            if (hier[id][2] == -1) continue;
            if (cv::contourArea(contours[id]) > 400) {
                int cnt = 0;
                int child = hier[id][2];
                while (child != -1) ++cnt, child = hier[child][0];
                if (cnt < 3) {
                    findLicensePlate(contours, hier, hier[id][2], plates);
                    continue;
                }
                cv::RotatedRect box = cv::minAreaRect(contours[id]);
                if (cv::contourArea(contours[id]) < box.size.area() * 0.6) {
                    findLicensePlate(contours, hier, hier[id][2], plates);
                    continue;
                }
                if (box.angle < -45.0)
                {
                    float tmp = box.size.width;
                    box.size.width = box.size.height;
                    box.size.height = tmp;
                    box.angle += 90.0f;
                }
                else if (box.angle > 45.0)
                {
                    float tmp = box.size.width;
                    box.size.width = box.size.height;
                    box.size.height = tmp;
                    box.angle -= 90.0f;
                }
                double whRatio = (double)box.size.width / box.size.height;
                if (!(3.0 < whRatio && whRatio < 10.0))
                {
                    findLicensePlate(contours, hier, hier[id][2], plates);
                    continue;
                }
                plates.push_back(id);
            }
        }
    }

    std::pair<cv::Mat, cv::Mat> preprocessImage(cv::Mat img){
        cv::Mat image = img;
        cv::Mat pyr, timg, gray0(image.size(), CV_8U), gray;
        pyrDown(image, pyr, cv::Size(image.cols / 2, image.rows / 2));
        pyrUp(pyr, timg, image.size());
        cv::cvtColor(timg, gray0, CV_BGR2GRAY);
        Canny(gray0, gray, 100, 50, 3);
//        cv::dilate(gray, gray, cv::Mat(), cv::Point(-1,-1));
        return std::make_pair(gray, gray0);
    }

    static const char* goodLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    bool notGoodLetter(char ch) {
        return strchr(goodLetters, ch) == 0;
    }

    void fixValue(std::string& value) {
        value.erase(std::remove_if(value.begin(), value.end(), notGoodLetter), value.end());
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
        vector< vector<cv::Point> > contours;
        vector<cv::Vec4i> hierarchy;
        vector<int> plates;

        std::pair<cv::Mat, cv::Mat> mats = preprocessImage(image);
        cv::Mat gray0 = mats.first;
        cv::findContours(gray0, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
        if (contours.size() > 0)
            findLicensePlate(contours, hierarchy, 0, plates);
        if (plates.size() == 0) return false;

//        cv::namedWindow("gray", CV_WINDOW_AUTOSIZE);

        value = "";
        for (size_t i = 0; i < plates.size(); ++i) {
            cv::drawContours(image, contours, plates[i], cv::Scalar(0, 0, 255), 2, 8, hierarchy, 0, cv::Point());
            cv::Rect plateRect = cv::boundingRect(contours[plates[i]]);
            cv::Mat letter2 = cv::Mat(mats.second, plateRect), rotated, letter1;
            cv::RotatedRect box = cv::minAreaRect(contours[plates[i]]);
            if (box.angle < -45.0)
            {
                float tmp = box.size.width;
                box.size.width = box.size.height;
                box.size.height = tmp;
                box.angle += 90.0f;
            }
            else if (box.angle > 45.0)
            {
                float tmp = box.size.width;
                box.size.width = box.size.height;
                box.size.height = tmp;
                box.angle -= 90.0f;
            }
            cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point(box.center.x - plateRect.x, box.center.y - plateRect.y), box.angle, 1);
            cv::warpAffine(letter2, rotated, rot_mat, letter2.size(), cv::INTER_CUBIC);
            cv::getRectSubPix(rotated, box.size, cv::Point(box.center.x - plateRect.x, box.center.y - plateRect.y), letter1);
            tessApi.SetImage((uchar*)letter1.data, letter1.cols, letter1.rows, letter1.channels(), letter1.step1());
            tessApi.SetRectangle(0, 0, letter1.cols, letter1.rows);
            value += tessApi.GetUTF8Text();
        }

        fixValue(value);
        return value.length() > 0;
    }

}

