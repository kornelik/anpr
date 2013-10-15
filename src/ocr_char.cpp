#include "ocr_char.h"

#include "util.h"
#include <fstream>

namespace anpr {

class OCRChar::Impl {
private:
    cv::KNearest oracle;

public:
	#define FEATURE_COUNT 10 * 16

    Impl(const std::string& learnpath, const std::string& allchars)
    {
		std::set<char> goodChars(allchars.begin(), allchars.end());
		std::ifstream train((learnpath + "/train.txt").c_str());
		std::vector< std::pair<char, std::string> > samples;
		char symbol;
		std::string imageFile;
		while (train >> symbol >> imageFile) {
			if (goodChars.find(symbol) == goodChars.end()) continue;
			samples.push_back(std::make_pair(symbol, imageFile));
		}
		cv::Mat trainData(samples.size(), FEATURE_COUNT, cv::DataType<float>::type);
        cv::Mat trainClasses(samples.size(), 1, cv::DataType<float>::type);
        for (size_t i = 0; i < samples.size(); ++i) {
			std::string path = learnpath + samples[i].second;
            cv::Mat inp = cv::imread(path, 0), out, canny;
			if (inp.empty()) continue;
			cv::Canny(inp, canny, 100, 50, 3);
			std::vector< std::vector<cv::Point> > contours;
			cv::findContours(canny, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
			int maxx = -1, maxy = -1, minx = 1e9, miny = 1e9;
			for (size_t j = 0; j < contours.size(); ++j) {
				cv::Rect r = cv::boundingRect(contours[j]);
				if (r.x + r.width > maxx) maxx = r.x + r.width;
				if (r.y + r.height > maxy) maxy = r.y + r.height;
				if (r.x < minx) minx = r.x;
				if (r.y < miny) miny = r.y;
			}
			cv::Rect bound(minx, miny, maxx - minx, maxy - miny);
	        cv::resize(cv::Mat(inp, bound), out, cv::Size(10, 16), 0, 0, cv::INTER_CUBIC);
			for (int j = 0; j < FEATURE_COUNT; ++j) trainData.at<float>(i, j) = out.data[j];
			trainClasses.at<float>(i, 0) = samples[i].first;
        }
		oracle.train(trainData, trainClasses);
    }

    char Classify(cv::Mat image) {
        cv::Mat bin;
        cv::adaptiveThreshold(image, image, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 0);
        cv::resize(image, bin, cv::Size(10, 16), 0, 0, cv::INTER_CUBIC);
        cv::Mat sample(1, FEATURE_COUNT, cv::DataType<float>::type);
        for (size_t i = 0; i < FEATURE_COUNT; ++i) sample.at<float>(0, i) = bin.data[i];
		char result = (char)(oracle.find_nearest(sample, 1) + 1e-5);
		return result;
    }
};


OCRChar::OCRChar(const std::string& learn_path, const std::string& possible_chars)
    : impl_(new Impl(learn_path, possible_chars)) {}

OCRChar::~OCRChar() {
    delete impl_;
}

char OCRChar::Classify(cv::Mat image) {
    return impl_->Classify(image);
}

}
