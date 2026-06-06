#define _USE_MATH_DEFINES

#include "SerialTransformation.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>

Image Grayscale(const Image& img) {
    if (img.channels != 3 && img.channels != 4) {
        throw std::runtime_error("Unsupported number of channels");
    }

    Image output;
    output.width = img.width;
    output.height = img.height;
    output.channels = 1;   // Grayscale image will have only one channel
    output.data.resize(img.width * img.height);

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            int src_idx = (y * img.width + x) * img.channels;
            unsigned char r = img.data[src_idx];
            unsigned char g = img.data[src_idx + 1];
            unsigned char b = img.data[src_idx + 2];

            unsigned char gray = static_cast<unsigned char>(
                0.299 * r + 0.587 * g + 0.114 * b
                );

            output.data[y * img.width + x] = gray;
        }
    }
    return output;
}


Image SobelEdgeDetection(const Image& grayImage, int threshold=100) {
    // Check to see if the image has only one channel (grayscale)
    if (grayImage.channels != 1) {
        throw std::runtime_error("Sobel edge detection requires grayscale image (channels=1)");
    }

    int width = grayImage.width;
    int height = grayImage.height;
    Image edgeImage;
    edgeImage.width = width;
    edgeImage.height = height;
    edgeImage.channels = 1;
    edgeImage.data.resize(width * height, 0);

    // Sobel masks (3x3)
    const int Gx[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
    const int Gy[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

    // Iterate through all non-border pixels
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int sumX = 0, sumY = 0;
            // Apply mask
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    unsigned char pixel = grayImage.data[(y + ky) * width + (x + kx)];
                    sumX += pixel * Gx[ky + 1][kx + 1];
                    sumY += pixel * Gy[ky + 1][kx + 1];
                }
            }
            int magnitude = std::abs(sumX) + std::abs(sumY); // ili sqrt(sumX*sumX + sumY*sumY)
            if (magnitude > threshold) {
                edgeImage.data[y * width + x] = 255;
            }
        }
    }
    return edgeImage;
}

std::vector<std::vector<int>> HoughTransform(const Image& edgeImage)
{
    if (edgeImage.channels != 1)
        throw std::runtime_error("Hough transformation requires an image with one channel pixels");

    int w = edgeImage.width;
    int h = edgeImage.height;

    double diagonal = std::sqrt(w * w + h * h);
    int rho_max = (int)std::ceil(diagonal);

    int theta_bins = 180;          // 0..179 degrees
    int rho_bins = 2 * rho_max + 1; // -rho_max to +rho_max

    // initialize accumulator (all zeroes)
    std::vector<std::vector<int>> accumulator(rho_bins, std::vector<int>(theta_bins, 0));

    // for every pixel
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            unsigned char pixel = edgeImage.data[y * w + x];
            if (pixel == 0) continue; // not edge

            for (int t_idx = 0; t_idx < theta_bins; ++t_idx) {
                double theta_deg = t_idx;          // 0..179 degrees
                double theta_rad = theta_deg * M_PI / 180.0;

                double rho = x * cos(theta_rad) + y * sin(theta_rad);
                int rho_idx = (int)round(rho) + rho_max; // index in the accumulator

                if (rho_idx >= 0 && rho_idx < rho_bins) {
                    accumulator[rho_idx][t_idx]++;
                }
            }
        }
    }

    return accumulator; 
}

std::vector<Line> DetectLines(const std::vector<std::vector<int>>& accumulator, int threshold)
{
    std::vector<Line> lines;
    if (accumulator.empty()) return lines;

    int rho_bins = accumulator.size();
    int theta_bins = accumulator[0].size();
    int rho_max = (rho_bins - 1) / 2;   //  rho_idx = rho + rho_max

    const double theta_step = M_PI / theta_bins; 

    // Pass through all cells of the accumulator 
    for (int r = 1; r < rho_bins - 1; ++r) {
        for (int t = 1; t < theta_bins - 1; ++t) {
            int value = accumulator[r][t];
            if (value < threshold) continue;

            // Check if local max (compare with 8 neighboors)
            bool is_max = true;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dt = -1; dt <= 1; ++dt) {
                    if (dr == 0 && dt == 0) continue;
                    if (accumulator[r + dr][t + dt] >= value) {
                        is_max = false;
                        break;
                    }
                }
                if (!is_max) break;
            }

            if (is_max) {
                double rho = r - rho_max;          
                double theta = t * theta_step;     
                lines.push_back({ rho, theta });
            }
        }
    }

    return lines;
}

