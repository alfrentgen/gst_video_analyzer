#ifndef _VIDEO_ANALYZER_WRAPPER_H_
#define _VIDEO_ANALYZER_WRAPPER_H_

#include <gst/gst.h>
#include "video_analyzer.h"

class VideoAnalyzerWrapper {
public:
    gint setModel(const gchar* model_path) {
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

void analyzeFrame(const GstVideoFrame* frame) {
    auto* pixels = GST_VIDEO_FRAME_PLANE_DATA(frame, 0);
    auto stride = GST_VIDEO_FRAME_PLANE_STRIDE (frame, 0);
    auto pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE (frame, 0);
    //analyzer.analyzeFrame(pixels, ,);
}

void drawMarkup(GstVideoFrame* frame) {
    ;
}

private:
    VideoAnalyzer analyzer;
    std::string exception_message;
};

#endif
