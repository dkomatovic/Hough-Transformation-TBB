#pragma once

#include "Structures.h"
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class Report
{
private:
	std::string Generate();
public:
	std::string processingMode;
	std::string imgTitle;
	Image img;
	long long grayscaleDuration;
	long long edgeDetectionDuration;
	long long houghTransformationDuration;
	long long lineDetectionDuration; 
	long long totalProcessingDuration;
	int accumulatorThreshold;
	std::vector<int> accumulatorHistogram;
	int maxAccumulatorVote;
	int detectedLinesNum;


	void Print();
	void Save(std::string path);
	void SaveHistogram(std::string path);
};

