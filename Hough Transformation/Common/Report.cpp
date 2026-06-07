#include "Report.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <matplot/matplot.h>

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

void Report::SaveHistogram(std::string path) {
    if (accumulatorHistogram.empty()) {
        std::cerr << "Histogram data empty." << std::endl;
        return;
    }

    // Prepare data for bar plot (votes vs number of cells)
    std::vector<double> x, y;
    for (int v = 0; v <= maxAccumulatorVote; ++v) {
        if (v < (int)accumulatorHistogram.size() && accumulatorHistogram[v] > 0) {
            x.push_back(static_cast<double>(v));
            y.push_back(static_cast<double>(accumulatorHistogram[v]));
        }
    }

    if (x.empty()) {
        std::cerr << "No positive histogram data." << std::endl;
        return;
    }

    // Create and save bar chart
    auto fig = matplot::figure(true);

    matplot::bar(x, y);
    matplot::xlabel("Number of votes");
    matplot::ylabel("Number of accumulator cells");
    matplot::title("Histogram of Hough Accumulator");
    matplot::ylim({ 0, 2000 });
    matplot::save(path);
    std::cout << "Histogram image saved to: " << path << std::endl;    
}
