#pragma once

#include <vector>

using namespace std;

struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;
    vector<unsigned char> data;
};

struct Line {
    double rho;
    double theta;
};