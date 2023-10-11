#include "video_analyzer.h"

namespace dnn = cv::dnn;

void VideoAnalyzer::setModel(const std::string& model_path) {
    try {
        net = cv::dnn::readNetFromONNX(model_path);
    } catch(const cv::Exception& e) {
        net.reset();
    }
    
    net->setPreferableBackend(cv::dnn::DNN_TARGET_CPU);
    net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

void VideoAnalyzer::highlightBoxes(cv::Mat& img, std::vector<Box>& boxes) {
    cv::Scalar rectColor(0,192,0);
    unsigned short fontScale = 2, confPrecis = 2;

    for (const Box& box: boxes) {
        std::string text = std::to_string(box.conf);
        cv::rectangle(img, {box.x1,box.y1}, {box.x2,box.y2}, rectColor, 2);
        cv::rectangle(
            img,
            {box.x1, box.y1 - fontScale * 12},
            {box.x1 + (unsigned short)text.length() * fontScale * 9, box.y1},
            rectColor,
            -1
        );
        cv::putText(img, text, {box.x1,box.y1}, cv::FONT_HERSHEY_PLAIN, fontScale, {255,255,255}, 2);
    }
}

void drawDetections(const std::vector<cv::Mat>& detections) {
    ;
}

void VideoAnalyzer::analyzeFrame(std::vector<uint8_t>& rgb_8bit_data, uint32_t width, uint32_t height, bool draw_detected) {
    if(net->empty()) {
        return;
    }
    auto image = cv::rawIn(rgb_8bit_data);
    auto blob = cv::dnn::blobFromImage(image, 1.0 / 255.0, cv::Size(width, height), cv::Scalar(0, 0, 0));

    net->setInput(blob);
    std::vector<cv::Mat> detections;
    auto outLayerNames = net->getUnconnectedOutLayersNames();
    net->forward(detections, outLayerNames);
    if (draw_detected) {
        drawDetections(detections);
    }
}
