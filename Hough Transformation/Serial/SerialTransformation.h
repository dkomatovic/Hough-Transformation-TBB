#pragma once

#include "ImageIO.h"
#include <vector>

Image Grayscale(const Image& img);
Image SobelEdgeDetection(const Image& grayImage, int threshold = 100);
std::vector<std::vector<int>> HoughTransform(const Image& edgeImage);
std::vector<Line> DetectLines(const std::vector<std::vector<int>>& accumulator, int threshold);

