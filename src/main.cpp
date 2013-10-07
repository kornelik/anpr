#include "opencv2/highgui/highgui_c.h"

int main()
{
	CvCapture* capture = cvCreateCameraCapture(CV_CAP_ANY);
	IplImage* frame = 0;

	cvNamedWindow("Camera", 0);
    cvResizeWindow("Camera", 640, 350);
	while (true)
	{
		frame = cvQueryFrame(capture);
		cvShowImage("Camera", frame);
		if ((cvWaitKey(10) & 255) == 27)
		{
			break;
		}
	}
	return 0;
}

