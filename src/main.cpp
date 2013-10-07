#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

using namespace cv;

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

