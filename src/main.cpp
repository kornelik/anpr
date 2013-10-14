#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#elif __linux__
#include "conio.h"
#endif

#include "recognizer.h"

int main(int argc, char* argv[]) {
	
	std::string answer;
	anpr::Recognizer* r = new anpr::Recognizer("train/");
	CvCapture* capture = cvCreateCameraCapture(CV_CAP_ANY);

	bool showGui = (argc > 1) && (!strcmp(argv[1], "--gui"));

	if (showGui) {
		cvNamedWindow("Camera", 0);
		cvResizeWindow("Camera", 640, 350);
	}
    
	while (true) {
		IplImage* frame = cvQueryFrame(capture);
		cv::Mat frameMat = frame;
		if (r->RecognizePlateNumber(frameMat, answer)) {
			std::cout << answer << std::endl;
		}
		if (showGui) {
			cv::imshow("Camera", frameMat);
			if ((cvWaitKey(10) & 255) == 27) {
				cvDestroyWindow("Camera");
				break;
			}
		}
		if (kbhit() && getch() == 32) {
			break;
		}
    }

	cvReleaseCapture(&capture);
	return 0;
}