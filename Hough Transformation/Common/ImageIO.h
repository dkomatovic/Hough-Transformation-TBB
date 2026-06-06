#pragma once

#include <vector>
#include <filesystem>
#include "Structures.h"

using namespace std;
namespace fs = std::filesystem;


vector<fs::path> FindInputImages(const fs::path& inputFolder);
Image LoadImage(const fs::path& path);
void SaveImage(const Image& img, const std::string& filename);