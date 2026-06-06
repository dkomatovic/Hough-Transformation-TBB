#pragma once

#include "Structures.h"
#include <string>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

class Report
{
private:
	string Generate();
public:
	string processingMode;
	string imgTitle;
	Image img;
	long long grayscaleDuration;
	long long edgeDetectionDuration;
	long long houghTransformationDuration;
	long long lineDetectionDuration; 
	long long totalProcessingDuration;
	int accumulatorThreshold;
	int detectedLinesNum;

	void Print();
	void Save(string path);
};

