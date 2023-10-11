#ifndef _VIDEO_ANALYZER_H_
#define _VIDEO_ANALYZER_H_

#include <stdio.h>
#include <opencv2/opencv.hpp>

class Box {
public:
    int x1, y1, x2, y2;
    float conf;
    Box(int x1, int y1, int x2,  int y2, float conf) :
        x1{x1},
        y1{y1},
        x2{x2},
        y2{y2},
        conf{conf}
    {}
};

namespace dnn = cv::dnn;

class VideoAnalyzer {
public:
    VideoAnalyzer() = default;
    ~VideoAnalyzer() = default;;
    
    void setModel(const std::string& model_path);
    void highlightBoxes(cv::Mat& img, std::vector<Box>& boxes);
    void analyzeFrame(std::vector<uint8_t>& rgb_8bit_data, uint32_t width, uint32_t height, bool draw_detected);
private:
    std::optional<dnn::Net> net;
};

#endif