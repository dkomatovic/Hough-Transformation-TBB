#pragma once

#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<unsigned char> data;
};

vector<fs::path> FindInputImages(const fs::path& inputFolder);
Image LoadImage(const fs::path& path);
void SaveImage(const Image& img, const std::string& filename);