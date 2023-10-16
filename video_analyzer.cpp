#include <iostream>

#include "video_analyzer.h"
#include "class_names.h"

void VideoAnalyzer::setModel(const std::string model_path) {
    try {
        m_net = cv::dnn::readNetFromDarknet(model_path + "/model.cfg", model_path + "/model.weights");
    } catch(const cv::Exception& e) {
        m_net.reset();
        throw e;
    }

    m_net->setPreferableBackend(cv::dnn::DNN_TARGET_CPU);
    m_net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

void drawDetection(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame)
{
    cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 255, 0), 2);

    std::string label = cv::format("%.2f", conf);
    if (!class_names.empty()) {
        label = (classId < class_names.size() ? class_names[classId] : "Unknown") + ": " + label;
    }

    int baseLine;
    cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    top = std::max(top, labelSize.height);
    rectangle(frame, cv::Point(left, top - labelSize.height), cv::Point(left + labelSize.width, top + baseLine), cv::Scalar::all(255), cv::FILLED);
    putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar());
}

void VideoAnalyzer::analyzeFrame(const std::vector<uchar>& rgb_8bit_data, uint32_t frame_width, uint32_t frame_height) {
    m_detections.clear();
    if(!m_net.has_value()) {
        return;
    }

    const double desired_width = 416;
    const double desired_height = 416;
    const double width_sf = frame_width / desired_width;
    const double height_sf = frame_height / desired_height;
    const auto scale_factor = std::min(height_sf, width_sf);
    auto frame = cv::Mat(frame_height, frame_width, CV_8UC3, (void*)rgb_8bit_data.data());
    auto blob = cv::dnn::blobFromImage(frame, 1.0 / 255, cv::Size(desired_width, desired_width), cv::Scalar(0,0,0), true, false);

    m_net->setInput(blob);
    auto out_layer_names = m_net->getUnconnectedOutLayersNames();
    std::vector<cv::Mat> detections;
    m_net->forward(detections, out_layer_names);

    static std::string outLayerType = m_net->getLayer(out_layer_names[0])->type;
    assert(outLayerType == "Region");
    if (outLayerType == "Region") {
        for (const auto& detection : detections) {
            // Network produces output blob with a shape NxC where N is a number of
            // detected objects and C is a number of classes + 4 where the first 4
            // numbers are [center_x, center_y, width, height]
            auto* data = (float*)detection.data;
            for (int j = 0; j < detection.rows; ++j, data += detection.cols)
            {
                cv::Mat scores = detection.row(j).colRange(5, detection.cols);
                cv::Point classIdPoint;
                double confidence;
                minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
                m_detections.push_back(detection_t {data[0], data[1], data[2], data[3], static_cast<float>(confidence), classIdPoint.x});
            }
        }
    }
}

const std::vector<detection_t>& VideoAnalyzer::getDetections() const {
    return m_detections;
}

void VideoAnalyzer::drawDetections(std::vector<uchar>& rgb_8bit_data, uint32_t frame_width, uint32_t frame_height, double conf_threshold) {
    auto frame = cv::Mat(frame_height, frame_width, CV_8UC3, (void*)rgb_8bit_data.data());
    for(const auto& d : m_detections) {
        auto& [center_x, center_y, width, height, conf, class_id] = d;
        if (conf > conf_threshold) {
            int c_x = (int)(center_x * frame_width);
            int c_y = (int)(center_y * frame_height);
            int w = (int)(width * frame_width);
            int h = (int)(height * frame_height);
            int x = c_x - (w / 2);
            int y = c_y - (h / 2);
            drawDetection(class_id, conf, x, y, x + w, y + h, frame);
        }
    }
}
