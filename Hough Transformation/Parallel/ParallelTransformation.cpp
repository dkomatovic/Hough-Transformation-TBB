#define _USE_MATH_DEFINES

#include "ParallelTransformation.h"
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/combinable.h>
#include <cmath>
#include <vector>
#include <algorithm>

Image Grayscale(const Image& img) {
    if (img.channels != 3 && img.channels != 4) {
        throw std::runtime_error("Unsupported number of channels");
    }

    Image output;
    output.width = img.width;
    output.height = img.height;
    output.channels = 1;   // Grayscale image will have only one channel
    output.data.resize(img.width * img.height);

    tbb::parallel_for(0, img.height, [&](int y) {
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
        });

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

    tbb::parallel_for(1, height - 1, [&](int y) {
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
        });

    return edgeImage;
}

std::vector<std::vector<int>> HoughTransform(const Image& edgeImage) {
    if (edgeImage.channels != 1)
        throw std::runtime_error("Hough transformation requires an image with one channel pixels");

    int w = edgeImage.width;
    int h = edgeImage.height;
    double diagonal = std::sqrt(w * w + h * h);
    int rho_max = static_cast<int>(std::ceil(diagonal));
    int theta_bins = 180;
    int rho_bins = 2 * rho_max + 1;
    int total_cells = rho_bins * theta_bins;

    // Thread‑local flat vectors
    tbb::combinable<std::vector<int>> local_accs;
    local_accs = tbb::combinable<std::vector<int>>([total_cells]() {
        return std::vector<int>(total_cells, 0);
        });

    tbb::parallel_for(0, h, [&](int y) {
        auto& local = local_accs.local();
        for (int x = 0; x < w; ++x) {
            unsigned char pixel = edgeImage.data[y * w + x];
            if (pixel == 0) continue;
            for (int t_idx = 0; t_idx < theta_bins; ++t_idx) {
                double theta_rad = t_idx * M_PI / 180.0;
                double rho = x * std::cos(theta_rad) + y * std::sin(theta_rad);
                int rho_idx = static_cast<int>(std::round(rho)) + rho_max;
                if (rho_idx >= 0 && rho_idx < rho_bins) {
                    int idx = rho_idx * theta_bins + t_idx;
                    local[idx]++;
                }
            }
        }
        });

    
    std::vector<int> global(total_cells, 0);
    local_accs.combine_each([&](const std::vector<int>& local) {
        for (size_t i = 0; i < local.size(); ++i)
            global[i] += local[i];
        });

    std::vector<std::vector<int>> result(rho_bins, std::vector<int>(theta_bins));
    for (int r = 0; r < rho_bins; ++r) {
        for (int t = 0; t < theta_bins; ++t) {
            result[r][t] = global[r * theta_bins + t];
        }
    }
    return result;
}

tbb::concurrent_vector<Line> DetectLines(const std::vector<std::vector<int>>& accumulator, int threshold) {

    int rho_bins = static_cast<int>(accumulator.size());
    int theta_bins = static_cast<int>(accumulator[0].size());
    int rho_max = (rho_bins - 1) / 2;
    const double theta_step = M_PI / theta_bins;

    tbb::concurrent_vector<Line> concurrent_lines;

    tbb::parallel_for(1, rho_bins - 1, [&](int r) {
        for (int t = 1; t < theta_bins - 1; ++t) {
            int value = accumulator[r][t];
            if (value < threshold) continue;

            bool is_max = true;
            for (int dr = -1; dr <= 1 && is_max; ++dr) {
                for (int dt = -1; dt <= 1; ++dt) {
                    if (dr == 0 && dt == 0) continue;
                    if (accumulator[r + dr][t + dt] >= value) {
                        is_max = false;
                        break;
                    }
                }
            }
            if (is_max) {
                double rho = r - rho_max;
                double theta = t * theta_step;
                concurrent_lines.push_back(Line{ rho, theta });
            }
        }
        });

    return concurrent_lines;
}


