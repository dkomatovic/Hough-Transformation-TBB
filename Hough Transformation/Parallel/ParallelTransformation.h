#pragma once
#include "Structures.h"
#include <tbb/concurrent_vector.h>

Image Grayscale(const Image& img);
Image SobelEdgeDetection(const Image& grayImage, int threshold);
std::vector<std::vector<int>> HoughTransform(const Image& edgeImage);
tbb::concurrent_vector<Line> DetectLines(const std::vector<std::vector<int>>& accumulator, int threshold);