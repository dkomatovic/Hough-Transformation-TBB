#pragma once

#include <vector>
#include <filesystem>
#include "Structures.h"

namespace fs = std::filesystem;

std::vector<fs::path> FindInputImages(const fs::path& inputFolder);
Image LoadImageFromFile(const fs::path& path);
void SaveImageToFile(const Image& img, const std::string& filename);