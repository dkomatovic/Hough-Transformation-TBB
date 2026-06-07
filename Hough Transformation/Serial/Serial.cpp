#include <iostream>
#include "ImageIO.h"
#include "Report.h"
#include "LineVisualization.h"
#include "SerialTransformation.h"
#include <vector>
#include <chrono>

namespace fs = std::filesystem;

int ProcessAccumulatorAndGetThreshold(const std::vector<std::vector<int>>& accumulator, Report& report) {
    int maxVal = 0;
    for (const auto& row : accumulator)
        for (int v : row)
            if (v > maxVal) maxVal = v;

    std::vector<int> hist(maxVal + 1, 0);
    for (const auto& row : accumulator)
        for (int v : row)
            hist[v]++;

    report.maxAccumulatorVote = maxVal;
    report.accumulatorHistogram = hist;

    return static_cast<int>(0.70 * maxVal);
}

void processImage(fs::path imgPath, std::string outputFolder) {
    
    // Starting total clock for report
    Report report;
    report.processingMode = "Serial";
    auto totalStart = std::chrono::steady_clock::now();

    // loading image
    Image img = LoadImageFromFile(imgPath);
    report.img = img;
    report.imgTitle = imgPath.stem().string();

    // grayscale phase
    auto start = std::chrono::steady_clock::now();
    Image grayImg = Grayscale(img);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    report.grayscaleDuration = duration;

    // edge detection phase
    start = std::chrono::steady_clock::now();
    Image edgesImg = SobelEdgeDetection(grayImg, 100);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    report.edgeDetectionDuration = duration;

    // hough transformation phase
    start = std::chrono::steady_clock::now();
    auto accumulator = HoughTransform(edgesImg);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    report.houghTransformationDuration = duration;

    // line detecting phase
    int threshold = ProcessAccumulatorAndGetThreshold(accumulator, report);
    report.accumulatorThreshold = threshold;

    start = std::chrono::steady_clock::now();
    auto lines = DetectLines(accumulator, threshold);
    end = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    report.lineDetectionDuration = duration;
    report.detectedLinesNum = lines.size();

    // visualizing lines
    Image imgWithLines = DrawLines(img, lines, 170, 0, 0);

    // Saving results to output folder
    std::string resultFolder = outputFolder + "/" + imgPath.stem().string();
    SaveImageToFile(grayImg, resultFolder + "/gray_" + imgPath.stem().string());
    SaveImageToFile(edgesImg, resultFolder + "/edge_" + imgPath.stem().string());
    SaveImageToFile(imgWithLines, resultFolder + "/final_" + imgPath.stem().string());
    auto totalEnd = std::chrono::steady_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(totalEnd - totalStart).count();
    report.totalProcessingDuration = totalDuration;

    report.Print();
    report.Save(resultFolder + "/report.txt");
    report.SaveHistogram(resultFolder + "/histogram.png");
}


int main()
{
    std::cout << "HOUGH TRANSFORMATION (SERIAL)\n";

    std::string inputDir = "../Input";
    std::string outputDir = "../OutputSerial";
    std::vector<fs::path> imgPaths = FindInputImages(inputDir);

    if (imgPaths.size() == 0) {
        std::cout << "No images found in the Input folder\n";
        return 0;
    }

    try {
        std::cout << "Choose the index of the image you would like to process: \n";
        for (size_t i = 0; i < imgPaths.size(); i++)
            std::cout << "[" << i << "] " << imgPaths[i].stem().string() << "\n";

        int index;
        fs::path chosenImgPath;
        while (true) {
            try {
                std::cout << "Index: ";
                std::cin >> index;

                if (std::cin.fail() || index < 0 || index >= static_cast<int>(imgPaths.size())) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid index. Please enter a number between 0 and " << imgPaths.size() - 1 << std::endl;
                }
                else {
                    chosenImgPath = imgPaths[index];
                    break;
                }
            }
            catch (std::exception e) {
                std::cout << "Invalid index. Try again" << std::endl;
            }
        }

        processImage(chosenImgPath, outputDir);
    }
    catch (std::exception e) {
        std::cout << e.what() << "\n";
    }
}
