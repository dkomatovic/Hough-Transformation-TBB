#include <iostream>
#include "ImageIO.h"
#include "Report.h"
#include "LineVisualization.h"
#include "ImageTransformation.h"
#include <vector>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;

int GetAccumulatorThreshold(const vector<vector<int>>& accumulator) {
    int maxVal = 0;
    for (const auto& row : accumulator)
        for (int v : row)
            if (v > maxVal) maxVal = v;

    return static_cast<int>(0.70 * maxVal);
}

void processImage(fs::path imgPath, string outputFolder) {
    
    // Starting total clock for report
    Report report;
    report.processingMode = "Serial";
    auto totalStart = chrono::steady_clock::now();

    // loading image
    Image img = LoadImage(imgPath);
    report.img = img;
    report.imgTitle = imgPath.stem().string();

    // grayscale phase
    auto start = chrono::steady_clock::now();
    Image grayImg = Grayscale(img);
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    report.grayscaleDuration = duration;

    // edge detection phase
    start = chrono::steady_clock::now();
    Image edgesImg = SobelEdgeDetection(grayImg);
    end = chrono::steady_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    report.edgeDetectionDuration = duration;

    // hough transformation phase
    start = chrono::steady_clock::now();
    auto accumulator = HoughTransform(edgesImg);
    end = chrono::steady_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    report.houghTransformationDuration = duration;

    // line detecting phase
    int threshold = GetAccumulatorThreshold(accumulator);
    report.accumulatorThreshold = threshold;

    start = chrono::steady_clock::now();
    auto lines = DetectLines(accumulator, threshold);
    end = chrono::steady_clock::now();
    duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    report.lineDetectionDuration = duration;
    report.detectedLinesNum = lines.size();

    // visualizing lines
    Image imgWithLines = DrawLines(img, lines, 170, 0, 0);

    // Saving results to output folder
    string resultFolder = outputFolder + "/" + imgPath.stem().string();
    SaveImage(grayImg, resultFolder + "/gray_" + imgPath.stem().string());
    SaveImage(edgesImg, resultFolder + "/edge_" + imgPath.stem().string());
    SaveImage(imgWithLines, resultFolder + "/final_" + imgPath.stem().string());
    auto totalEnd = chrono::steady_clock::now();
    auto totalDuration = chrono::duration_cast<chrono::milliseconds>(totalEnd - totalStart).count();
    report.totalProcessingDuration = totalDuration;

    report.Print();
    report.Save(resultFolder + "/report.txt");
    // dodaj tekstualni fajl za prikaz
}


int main()
{
    string inputDir = "../Input";
    string outputDir = "../OutputSerial";
    vector<fs::path> imgPaths = FindInputImages(inputDir);

    if (imgPaths.size() == 0) {
        cout << "No images found in the Input folder" << endl;
        return 0;
    }

    try {
        cout << "Choose the index of the image you would like to process: " << endl;
        for (size_t i = 0; i < imgPaths.size(); i++)
            cout << "[" << i << "] " << imgPaths[i].stem().string() << endl;

        int index;
        fs::path chosenImgPath;
        while (true) {
            try {
                cout << "Index: ";
                cin >> index;

                if (cin.fail() || index < 0 || index >= static_cast<int>(imgPaths.size())) {
                    cin.clear();  
                    cin.ignore(std::numeric_limits<streamsize>::max(), '\n'); 
                    cout << "Invalid index. Please enter a number between 0 and " << imgPaths.size() - 1 << std::endl;
                }
                else {
                    chosenImgPath = imgPaths[index];
                    break;
                }
            }
            catch (exception e) {
                cout << "Invalid index. Try again" << endl;
            }
        }

        processImage(chosenImgPath, outputDir);
    }
    catch (exception e) {
        cout << e.what() << endl;
    }
}
