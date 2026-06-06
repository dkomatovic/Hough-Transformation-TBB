#include "Report.h"
#include <iostream>
#include <sstream>
#include <fstream>

std::string Report::Generate() {
    std::ostringstream oss;
    oss << "========================================\n";
    oss << "HOUGH TRANSFORM ANALYSIS REPORT\n";
    oss << "========================================\n";
    oss << "Processing mode: " << processingMode << "\n";
    oss << "Image title: " << imgTitle << "\n";
    oss << "Image dimensions: " << img.width << " x " << img.height << ", channels: " << (int)img.channels << "\n";
    oss << "Accumulator threshold: " << accumulatorThreshold << "\n\n";
    oss << "Timings (milliseconds):\n";
    oss << "  Grayscale: " << grayscaleDuration << " ms\n";
    oss << "  Edge detection: " << edgeDetectionDuration << " ms\n";
    oss << "  Hough transform: " << houghTransformationDuration << " ms\n";
    oss << "  Line detection: " << lineDetectionDuration << " ms\n";
    oss << "\n(along with loading, and saving)\nTotal duration: " << totalProcessingDuration << " ms\n";
    oss << "\nNumber of lines detected: " << detectedLinesNum << "\n";
    oss << "========================================\n";
    return oss.str();
}

void Report::Print()
{
    std::cout << Generate();
}

void Report::Save(std::string path)
{
    std::ofstream out(path);
    if (out.is_open()) {
        out << Generate();
        out.close();
        std::cout << "Report saved to: " << path << std::endl;
    }
    else {
        std::cerr << "Error: Could not save report to " << path << std::endl;
    }
}
