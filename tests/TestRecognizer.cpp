#include <opencv2/highgui/highgui.hpp>
#include "TestRecognizer.h"

using namespace std;
using namespace cv;

#define THRESHOLD 0.7
#define PLATES_PATH "tests/"

static const char* plates[]  = {"1.jpg", "2.jpg", "3.jpg", "4.jpg"};
static const char* numbers[] = {"S6436", "6827IA5", "1188MH7", "6969MX7"};
static const char* emptys[]  = {"5.jpg", "6.jpg"};

CPPUNIT_TEST_SUITE_REGISTRATION(TestRecognizer);

bool TestRecognizer::threshold(const char* guess, const char* exact) {

	int guessSize = strlen(guess),
		exactSize = strlen(exact);
	
	int min = guessSize < exactSize
			? guessSize : exactSize;

	int matched = 0;
	for (int i = 0; i < min; i++) {
		if (guess[i] == exact[i]) {
			matched++;
		}
	}

	return (float(matched) / float(exactSize)) >= THRESHOLD;
}

void TestRecognizer::testRecognizePlateNumber() {
	string number;
	int count = sizeof(plates) / sizeof(const char*);
	for (int i = 0; i < count; i++) {
		Mat plate = imread(string(PLATES_PATH) + plates[i]);
		CPPUNIT_ASSERT(r.RecognizePlateNumber(plate, number));
		CPPUNIT_ASSERT(threshold(number.c_str(), numbers[i]));
	}
	count = sizeof(emptys) / sizeof(const char*);
	for (int i = 0; i < count; i++) {
		Mat plate = imread(string(PLATES_PATH) + emptys[i]);
		CPPUNIT_ASSERT(!r.RecognizePlateNumber(plate, number));
	}
}