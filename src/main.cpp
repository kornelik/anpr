#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "recognizer.h"

int main()
{
    std::string answer;
    anpr::Recognizer r;
    CvCapture* capture = cvCreateCameraCapture(CV_CAP_ANY);

    cvNamedWindow("Camera", 0);
    cvResizeWindow("Camera", 640, 350);
    while (true)
    {
        IplImage* frame = cvQueryFrame(capture);
        cv::Mat frameMat = frame;
        if (r.RecognizePlateNumber(frameMat, answer)) {
            std::cout << answer << std::endl;
        }
        cv::imshow("Camera", frameMat);
        if ((cvWaitKey(10) & 255) == 27)
        {
            break;
        }
    }

    return 0;
}

