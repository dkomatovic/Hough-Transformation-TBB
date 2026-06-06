#define _CRT_SECURE_NO_WARNINGS   // for suppressing sprintf warning
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "ImageIO.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

std::vector<fs::path> FindInputImages(const fs::path& inputFolder) {
    std::vector<fs::path> imgPaths;

    const std::vector<std::string> allowedExtensions = {
        ".bmp", ".png", ".ppm", ".jpg"
    };

    if (fs::exists(inputFolder)) {
        for (const auto& entry : fs::directory_iterator(inputFolder)) {
            if (!fs::is_regular_file(entry.status())) continue;

            std::string ext = entry.path().extension().string();
            
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return std::tolower(c); });

            if (std::find(allowedExtensions.begin(), allowedExtensions.end(), ext) != allowedExtensions.end()) {
                imgPaths.push_back(entry.path());
            }
        }
    }

    return imgPaths;
}

Image LoadImageFromFile(const fs::path& path) {
    Image img;
    int width, height, channels;
    unsigned char* data = stbi_load(path.string().c_str(),
        &width, &height, &channels, 0);
    if (data) {
        img.width = width;
        img.height = height;
        img.channels = channels;
        img.data.assign(data, data + (width * height * channels));
        stbi_image_free(data);
    }
    else {
        throw std::runtime_error("Failed to load image: " + path.string());
    }
    return img;
}

void SaveImageToFile(const Image& img, const std::string& filename) {
    // Validation
    if (img.data.empty() || img.width == 0 || img.height == 0) {
        throw std::runtime_error("Cannot save empty image");
    }
    if (img.channels != 1 && img.channels != 3 && img.channels != 4) {
        throw std::runtime_error("Unsupported number of channels");
    }

    // Create if it doesn't exist
    fs::path p(filename);
    fs::create_directories(p.parent_path());

    // Every output is .png
    std::string out_filename = filename;
    size_t dot = out_filename.find_last_of('.');
    if (dot == std::string::npos || out_filename.substr(dot) != ".png") {
        out_filename += ".png";
    }

    int success = stbi_write_png(out_filename.c_str(),
        img.width, img.height, img.channels,
        img.data.data(), img.width * img.channels);
    if (!success) {
        throw std::runtime_error("Failed to save image: " + out_filename);
    }
}