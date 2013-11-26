#include <opencv2/highgui/highgui.hpp>
#include "../src/recognizer.h"
#include "TestRecognizer.h"

using namespace std;
using namespace cv;

#define THRESHOLD 0.8
#define PLATES_PATH "tests/"
#define SAMPLE_PATH "../src/train/"

/* Plates pictures names */
static const char* plates[]  = {
	"IMAG1234.jpg",
	"IMAG5678.jpg",
	"IMAG9182.jpg",
	"IMAG0593.jpg",
    "IMAG0594.jpg",
    "IMAG0595.jpg",
    "IMAG0596.jpg",
    "IMAG0597.jpg",
    "IMAG0598.jpg",
    "IMAG0600.jpg",
    "IMAG0604.jpg",
    "IMAG0608.jpg",
    "IMAG0609.jpg",
    "IMAG0610.jpg",
    "IMAG0612.jpg",
    "IMAG0613.jpg",
};

/* Plates pictures numbers */
static const char* numbers[] = {
	"6827IA5",
	"1188MH7",
	"6969MX7",
	"8700EP7",
    "6827IA5",
    "6969MX7",
    "1501BT7",
    "1188MH7",
    "7778MB7",
    "0689MK7",
    "5617IP7",
    "8894OT7",
    "7021OH7",
    "3841MA7",
    "0057IM7",
    "7977OA7",
};

/* Pictures with no plates */
static const char* emptys[]  = {
	"empty1.jpg",
	"empty2.jpg",
};

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
	anpr::Recognizer* r = new anpr::Recognizer(SAMPLE_PATH);
	int count = sizeof(plates) / sizeof(const char*);
	for (int i = 0; i < count; i++) {
		Mat plate = imread(string(PLATES_PATH) + plates[i]);
		CPPUNIT_ASSERT(r->RecognizePlateNumber(plate, number));
		bool passed = threshold(number.c_str(), numbers[i]);
		CPPUNIT_ASSERT(passed);
		printf("\n%02d: OK", i + 1);
	}
	count = sizeof(emptys) / sizeof(const char*);
	for (int i = 0; i < count; i++) {
		Mat plate = imread(string(PLATES_PATH) + emptys[i]);
		CPPUNIT_ASSERT(!r->RecognizePlateNumber(plate, number));
	}
    delete r;
}
