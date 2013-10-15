#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include "recognizer.h"

const char * test_images[] = {
    "IMAG0593.jpg",
    "IMAG0594.jpg",
    "IMAG0595.jpg",
    "IMAG0596.jpg",
    "IMAG0597.jpg",
    "IMAG0598.jpg",
    "IMAG0599.jpg",
    "IMAG0600.jpg",
    "IMAG0604.jpg",
    "IMAG0608.jpg",
    "IMAG0609.jpg",
    "IMAG0610.jpg",
    "IMAG0612.jpg",
    "IMAG0613.jpg"
};

const char * test_result[] = {
    "8700EP7",
    "6827IA5",
    "6969MX7",
    "1501BT7",
    "1188MH7",
    "7778MB7",
    "7187KX7",
    "0689MK7",
    "5617IP7",
    "8894OT7",
    "7021OH7",
    "3841MA7",
    "0057IM7",
    "7977OA7"
};

template <typename T>
typename T::size_type levenshtein_distance(const T & src, const T & dst) {
    const typename T::size_type m = src.size();
    const typename T::size_type n = dst.size();
    if (m == 0) return n;
    if (n == 0) return m;
    std::vector< std::vector<typename T::size_type> > matrix(m + 1);
    for (typename T::size_type i = 0; i <= m; ++i) {
        matrix[i].resize(n + 1);
        matrix[i][0] = i;
    }
    for (typename T::size_type i = 0; i <= n; ++i) {
        matrix[0][i] = i;
    }
    typename T::size_type above_cell, left_cell, diagonal_cell, cost;
    for (typename T::size_type i = 1; i <= m; ++i) {
        for(typename T::size_type j = 1; j <= n; ++j) {
            cost = src[i - 1] == dst[j - 1] ? 0 : 1;
            above_cell = matrix[i - 1][j];
            left_cell = matrix[i][j - 1];
            diagonal_cell = matrix[i - 1][j - 1];
            matrix[i][j] = std::min(std::min(above_cell + 1, left_cell + 1), diagonal_cell + cost);
        }
    }
    return matrix[m][n];
}

int main()
{
    try {
        std::string v;
        anpr::Recognizer r("train/");

        size_t errorPrecision = 0;
        size_t errorRecall = 0;

        int test_images_count = sizeof(test_images) / sizeof(test_images[0]);
        for (int testi = 0; testi < test_images_count; ++testi) {
            cv::Mat img = cv::imread(std::string("tests/") + test_images[testi]);

            std::cout << "Answer: " << std::setw(10) << test_result[testi] << ", Got: ";
            v = "";
            if (r.RecognizePlateNumber(img, v)) {
                std::cout << std::setw(10) << v;

                size_t curError = levenshtein_distance(v, std::string(test_result[testi]));
                std::cout << ", Error: " << curError << std::endl;
                errorPrecision += curError;
            } else {
                std::cout << std::setw(10) << "" << ", Not found" << std::endl;
                ++errorRecall;
            }
        }

        std::cout << "Precision error: " << errorPrecision << std::endl;
        std::cout << "Recall error: " << double(errorRecall * 100) / test_images_count << "%" << std::endl;
    } catch(std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
    }
    return 0;
}

