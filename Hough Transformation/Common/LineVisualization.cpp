#include "LineVisualization.h"
#include <algorithm>

void setPixel(Image& img, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= img.width || y < 0 || y >= img.height) return;
    int idx = (y * img.width + x) * img.channels;
    if (img.channels == 3) {
        img.data[idx] = r;
        img.data[idx + 1] = g;
        img.data[idx + 2] = b;
    }
    else if (img.channels == 1) {
        // turn rgb parameter to grayscale 
        unsigned char gray = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        img.data[idx] = gray;
    }
}

void drawLine(Image& img, int x0, int y0, int x1, int y1, unsigned char r, unsigned char g, unsigned char b) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (true) {
        setPixel(img, x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

std::vector<std::pair<int, int>> getLineEndpoints(double rho, double theta, int width, int height) {
    std::vector<std::pair<int, int>> points;
    double cos_t = cos(theta);
    double sin_t = sin(theta);

    // Intersection with y = 0
    if (fabs(sin_t) > 1e-6) {
        int x = (int)round((rho - 0 * sin_t) / cos_t);
        if (x >= 0 && x < width) points.emplace_back(x, 0);
    }
    // Intersection with y = height-1
    if (fabs(sin_t) > 1e-6) {
        int x = (int)round((rho - (height - 1) * sin_t) / cos_t);
        if (x >= 0 && x < width) points.emplace_back(x, height - 1);
    }
    // Intersection with x = 0
    if (fabs(cos_t) > 1e-6) {
        int y = (int)round((rho - 0 * cos_t) / sin_t);
        if (y >= 0 && y < height) points.emplace_back(0, y);
    }
    // Intersection with x = width-1
    if (fabs(cos_t) > 1e-6) {
        int y = (int)round((rho - (width - 1) * cos_t) / sin_t);
        if (y >= 0 && y < height) points.emplace_back(width - 1, y);
    }

    // Remove duplicates and return two points
    if (points.size() < 2) return {};
    sort(points.begin(), points.end());
    points.erase(unique(points.begin(), points.end()), points.end());
    if (points.size() >= 2) return { points[0], points[1] };
    return {};
}

Image DrawLines(const Image& img, std::vector<Line> lines,
    unsigned char r, unsigned char g, unsigned char b)
{
    Image result = img;

    // If grayscale turn into rgb
    if (result.channels == 1) {
        Image colorImg;
        colorImg.width = result.width;
        colorImg.height = result.height;
        colorImg.channels = 3;
        colorImg.data.resize(result.width * result.height * 3);
        for (size_t i = 0; i < result.data.size(); ++i) {
            colorImg.data[i * 3] = result.data[i];
            colorImg.data[i * 3 + 1] = result.data[i];
            colorImg.data[i * 3 + 2] = result.data[i];
        }
        result = std::move(colorImg);
    }

    // Draw each line
    for (const auto& line : lines) {
        auto endpoints = getLineEndpoints(line.rho, line.theta, result.width, result.height);
        if (endpoints.size() >= 2) {
            drawLine(result,
                endpoints[0].first, endpoints[0].second,
                endpoints[1].first, endpoints[1].second,
                r, g, b);
        }
    }

    return result;
}