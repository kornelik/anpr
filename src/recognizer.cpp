#include "recognizer.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vector>
#include <cstdio>

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

    cv::Mat preprocessImage(cv::Mat img){
        cv::Mat image = img;
        cv::Mat pyr, timg, gray0(image.size(), CV_8U), gray;
        pyrDown(image, pyr, cv::Size(image.cols / 2, image.rows / 2));
        pyrUp(pyr, timg, image.size());
        cv::cvtColor(timg, gray0, CV_BGR2GRAY);
        Canny(gray0, gray, 100, 50, 3);
//        cv::dilate(gray, gray, cv::Mat(), cv::Point(-1,-1));
        return gray;
    }
}

namespace anpr {

    Recognizer::Recognizer() {
        tessApi.Init(NULL, "eng");
        tessApi.SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
        tessApi.SetVariable("tessedit_char_whitelist", "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    }

    bool Recognizer::RecognizePlateNumber(cv::Mat image, std::string& value) {
        vector< vector<cv::Point> > contours;
        vector<cv::Vec4i> hierarchy;
        vector<int> plates;

        cv::Mat gray0 = preprocessImage(image);
        cv::findContours(gray0, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
        findLicensePlate(contours, hierarchy, 0, plates);
        if (plates.size() == 0) return false;

        //cv::namedWindow("gray", CV_WINDOW_AUTOSIZE);
        //cv::imshow("gray", gray0);
        //cv::waitKey(0);

        for (size_t i = 0; i < plates.size(); ++i) {
            cv::drawContours(image, contours, plates[i], cv::Scalar(0, 0, 255), 2, 8, hierarchy, 0, cv::Point());
            cv::Rect plateRect = cv::boundingRect(contours[plates[i]]);
            for (int id = hierarchy[plates[i]][2]; id != -1; id = hierarchy[id][0]) {
                cv::Rect letterRect = cv::boundingRect(contours[id]);
                if (letterRect.height > plateRect.height * 0.5) {
                    cv::drawContours(image, contours, id, cv::Scalar(0, 255, 0), 1, 8, hierarchy, 0, cv::Point());
                }
            }
        }

        value = "";
        return true;
    }

}

