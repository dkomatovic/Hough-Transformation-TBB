#define _USE_MATH_DEFINES

#include "ImageTransformation.h"
#include <cmath>

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

#include <cmath>
#include <algorithm>
#include <stdexcept>

Image SobelEdgeDetection(const Image& grayImage, int threshold) {
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

vector<vector<int>> HoughTransform(const Image& edgeImage)
{
    if (edgeImage.channels != 1)
        throw runtime_error("Hough transformation requires an image with one channel pixels");

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
            if (pixel == 0) continue; // nije ivica

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

vector<Line> DetectLines(const vector<vector<int>>& accumulator, int threshold)
{
    std::vector<Line> lines;
    if (accumulator.empty()) return lines;

    int rho_bins = accumulator.size();
    int theta_bins = accumulator[0].size();
    int rho_max = (rho_bins - 1) / 2;   // pošto rho_idx = rho + rho_max

    const double theta_step = M_PI / theta_bins; // radijani po bin-u (180° -> π rad)

    // Prolaz kroz sve ćelije akumulatora (preskačemo ivice radi jednostavnije provere)
    for (int r = 1; r < rho_bins - 1; ++r) {
        for (int t = 1; t < theta_bins - 1; ++t) {
            int value = accumulator[r][t];
            if (value < threshold) continue;

            // Provera da li je lokalni maksimum (poređenje sa 8 suseda)
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
                double rho = r - rho_max;          // stvarno ρ
                double theta = t * theta_step;     // u radijanima
                lines.push_back({ rho, theta });
            }
        }
    }

    return lines;
}

void setPixel(Image& img, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= img.width || y < 0 || y >= img.height) return;
    int idx = (y * img.width + x) * img.channels;
    if (img.channels == 3) {
        img.data[idx] = r;
        img.data[idx + 1] = g;
        img.data[idx + 2] = b;
    }
    else if (img.channels == 1) {
        // Ako crtamo na grayscale, pretvori boju u sivu nijansu
        unsigned char gray = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        img.data[idx] = gray;
    }
    // za channels 4 (RGBA) – možeš dodati Alpha = 255
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

    // Presek sa y = 0
    if (fabs(sin_t) > 1e-6) {
        int x = (int)round((rho - 0 * sin_t) / cos_t);
        if (x >= 0 && x < width) points.emplace_back(x, 0);
    }
    // Presek sa y = height-1
    if (fabs(sin_t) > 1e-6) {
        int x = (int)round((rho - (height - 1) * sin_t) / cos_t);
        if (x >= 0 && x < width) points.emplace_back(x, height - 1);
    }
    // Presek sa x = 0
    if (fabs(cos_t) > 1e-6) {
        int y = (int)round((rho - 0 * cos_t) / sin_t);
        if (y >= 0 && y < height) points.emplace_back(0, y);
    }
    // Presek sa x = width-1
    if (fabs(cos_t) > 1e-6) {
        int y = (int)round((rho - (width - 1) * cos_t) / sin_t);
        if (y >= 0 && y < height) points.emplace_back(width - 1, y);
    }

    // Ukloni duplikate i vrati dve tačke (ako postoje)
    if (points.size() < 2) return {};
    // Sortiraj i ukloni duplikate (iste tačke)
    std::sort(points.begin(), points.end());
    points.erase(std::unique(points.begin(), points.end()), points.end());
    if (points.size() >= 2) return { points[0], points[1] };
    return {};
}

Image DrawLines(const Image& img, std::vector<Line> lines,
    unsigned char r, unsigned char g, unsigned char b)
{
    // Kreiraj radnu kopiju slike
    Image result = img;

    // Ako je izvorna slika grayscale, pretvori je u RGB (3 kanala)
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

    // Provera (opciono)
    if (result.channels < 3) {
        throw std::runtime_error("Image must have 1 or 3 channels to draw lines");
    }

    // Crtanje svake linije
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