#include <iostream>

#include "video_analyzer.h"
#include "class_names.h"

void VideoAnalyzer::setModel(const std::string model_path) {
    try {
        net = cv::dnn::readNetFromDarknet(model_path + "/model.cfg", model_path + "/model.weights");
    } catch(const cv::Exception& e) {
        net.reset();
        throw e;
    }

    net->setPreferableBackend(cv::dnn::DNN_TARGET_CPU);
    net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

void drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame)
{
    using namespace cv;
    rectangle(frame, cv::Point(left, top), Point(right, bottom), Scalar(0, 255, 0));

    std::string label = format("%.2f", conf);
    if (!class_names.empty())
    {
        label = (classId < class_names.size() ? class_names[classId] : "Unknown") + ": " + label;
    }

    int baseLine;
    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    top = max(top, labelSize.height);
    rectangle(frame, Point(left, top - labelSize.height),
              Point(left + labelSize.width, top + baseLine), Scalar::all(255), FILLED);
    putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.5, Scalar());
}

void VideoAnalyzer::analyzeFrame(std::vector<uchar>& rgb_8bit_data, uint32_t frame_width, uint32_t frame_height) {
    if(!net.has_value()) {
        return;
    }

    const double desired_width = 416;
    const double desired_height = 416;
    const double width_sf = frame_width / desired_width;
    const double height_sf = frame_height / desired_height;
    const auto scale_factor = std::min(height_sf, width_sf);
    auto frame = cv::Mat(frame_height, frame_width, CV_8UC3, (void*)rgb_8bit_data.data());
    auto blob = cv::dnn::blobFromImage(frame, 1.0 / 255, cv::Size(desired_width, desired_width), cv::Scalar(0,0,0), true, false);

    net->setInput(blob);
    auto out_layer_names = net->getUnconnectedOutLayersNames();
    std::vector<cv::Mat> detections;
    net->forward(detections, out_layer_names);

    static std::string outLayerType = net->getLayer(out_layer_names[0])->type;
    assert(outLayerType == "Region");
    if (outLayerType == "Region") {
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
        for (const auto& detection : detections) {
            // Network produces output blob with a shape NxC where N is a number of
            // detected objects and C is a number of classes + 4 where the first 4
            // numbers are [center_x, center_y, width, height]
            float* data = (float*)detection.data;
            for (int j = 0; j < detection.rows; ++j, data += detection.cols)
            {
                cv::Mat scores = detection.row(j).colRange(5, detection.cols);
                cv::Point classIdPoint;
                double confidence;
                minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
                if (confidence > 0.5)
                {
                    int centerX = (int)(data[0] * frame_width);
                    int centerY = (int)(data[1] * frame_height);
                    int width = (int)(data[2] * frame_width);
                    int height = (int)(data[3] * frame_height);
                    int left = centerX - (width / 2);
                    int top = centerY - (height / 2);

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }
        for (size_t idx = 0; idx < boxes.size(); ++idx) {
            auto& box = boxes[idx];
            drawPred(classIds[idx], confidences[idx], box.x, box.y,
                    box.x + box.width, box.y + box.height, frame);
        }
    }
}
