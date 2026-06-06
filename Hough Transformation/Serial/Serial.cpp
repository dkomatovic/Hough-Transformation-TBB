#include <iostream>
#include "ImageIO.h"
#include "ImageTransformation.h"
#include <vector>

using namespace std;
namespace fs = std::filesystem;

void processImage(fs::path imgPath, string outputFolder) {
    // Time start 
    Image img = LoadImage(imgPath);
    Image grayImg = Grayscale(img);
    Image edgesImg = SobelEdgeDetection(grayImg);
    auto accumulator = HoughTransform(edgesImg);
    auto lines = DetectLines(accumulator, 450);
    cout << "Broj linija: " << lines.size() << endl;
    Image imgWithLines = DrawLines(img, lines, 170, 0, 0);

    SaveImage(grayImg, outputFolder + "/gray_" + imgPath.stem().string());
    SaveImage(edgesImg, outputFolder + "/edge_" + imgPath.stem().string());
    SaveImage(imgWithLines, outputFolder + "/final_" + imgPath.stem().string());

    // Time end
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
        cout << "Choose which image to process: " << endl;
        for (auto imgPath : imgPaths) {
            processImage(imgPath, outputDir);
        }
    }
    catch (exception e) {
        cout << e.what() << endl;
    }
}
