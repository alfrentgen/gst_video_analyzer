#ifndef _VIDEO_ANALYZER_WRAPPER_H_
#define _VIDEO_ANALYZER_WRAPPER_H_

#include <cstring>
#include <gst/gst.h>
#include "video_analyzer.h"

class VideoAnalyzerWrapper {
public:
    gboolean setModel(const gchar* model_path) {
        try {
            analyzer.setModel(std::string(model_path));
        } catch(const cv::Exception& e) {
            exception_message = e.what();
            return FALSE;
        }
        return TRUE;
    }
    
    const gchar* getExceptionMessage() {
        return exception_message.c_str();
    }

    void analyzeFrame(const GstVideoFrame* frame, gboolean draw_detection) {
        auto* pixels = reinterpret_cast<uint8_t*>(GST_VIDEO_FRAME_PLANE_DATA(frame, 0));
        auto stride = GST_VIDEO_FRAME_PLANE_STRIDE (frame, 0);
        auto width = GST_VIDEO_FRAME_WIDTH(frame);
        auto height = GST_VIDEO_FRAME_HEIGHT(frame);
        assert(GST_VIDEO_FRAME_FORMAT(frame) == GST_VIDEO_FORMAT_RGB);
        std::vector<uchar> pixel_vector;
        pixel_vector.reserve(stride * height);
        pixel_vector.insert(pixel_vector.begin(), pixels, pixels + stride * height);
        analyzer.analyzeFrame(pixel_vector, width, height);
        if (draw_detection) {
            analyzer.drawDetections(pixel_vector, width, height);
            std::memcpy(pixels, pixel_vector.data(), pixel_vector.size());
        }
    }

    const std::vector<detection_t> getDetections() const {
        return analyzer.getDetections();
    }

    private:
        VideoAnalyzer analyzer;
        std::string exception_message;
    };

#endif
