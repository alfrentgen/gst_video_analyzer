#ifndef _VIDEO_ANALYZER_H_
#define _VIDEO_ANALYZER_H_

#include <cinttypes>
#include <optional>
#include <opencv2/opencv.hpp>

typedef struct
{
    float center_x;
    float center_y;
    float width;
    float height;
    float confidence;
    int class_id;
} detection_t;

class VideoAnalyzer
{
public:
    VideoAnalyzer()
    {
        m_net.reset();
    };
    ~VideoAnalyzer() = default;

    void setModel(const std::string model_dir);
    void analyzeFrame(const std::vector<uchar>& rgb_8bit_data, uint32_t frame_width, uint32_t frame_height);
    const std::vector<detection_t>& getDetections() const;
    void drawDetections(std::vector<uchar>& rgb_8bit_data, uint32_t frame_width, uint32_t frame_height, double conf_threshold = 0.33);

private:
    uint32_t m_input_width{};
    uint32_t m_input_height{};
    std::optional<cv::dnn::Net> m_net;
    std::vector<detection_t> m_detections;
};

#endif