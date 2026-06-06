#pragma once

#include "ImageIO.h"
#include <vector>

using namespace std;

Image Grayscale(const Image& img);
Image SobelEdgeDetection(const Image& grayImage, int threshold = 100);
vector<vector<int>> HoughTransform(const Image& edgeImage);
vector<Line> DetectLines(const vector<vector<int>>& accumulator, int threshold);
Image DrawLines(const Image& img, vector<Line> lines, unsigned char r, unsigned char g, unsigned char b);

