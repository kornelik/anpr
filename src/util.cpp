#include <opencv2/opencv.hpp>

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

}
