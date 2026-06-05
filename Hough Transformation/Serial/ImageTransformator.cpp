#include "ImageTransformator.h"

Image ImageTransformator::Grayscale(const Image& img) {
    if (img.channels != 3 && img.channels != 4) {
        throw std::runtime_error("Unsupported number of channels");
    }

    Image output;
    output.width = img.width;
    output.height = img.height;
    output.channels = img.channels;   // keeping the original number of channels
    output.data.resize(img.data.size()); 

    for (size_t i = 0; i < img.data.size(); i += img.channels) {
        // rgb channels
        unsigned char r = img.data[i];
        unsigned char g = img.data[i + 1];
        unsigned char b = img.data[i + 2];

        unsigned char gray = static_cast<unsigned char>(
            0.299 * r + 0.587 * g + 0.114 * b
            );

        output.data[i] = gray;
        output.data[i + 1] = gray;
        output.data[i + 2] = gray;

        // If there is an alpha (4. channel), it remains the same
        if (img.channels == 4) {
            output.data[i + 3] = img.data[i + 3];
        }
    }
    return output;
}