#ifndef _VIDEO_ANALYZER_WRAPPER_H_
#define _VIDEO_ANALYZER_WRAPPER_H_

#include <cstring>
#include <exception>
#include <gst/gst.h>

#include "video_analyzer.h"
#include "detection_meta.h"

class VideoAnalyzerWrapper
{
public:
    gboolean setModel(const gchar* model_path)
    {
        try {
            analyzer.setModel(std::string(model_path));
        }
        catch (const std::exception& e) {
            exception_message = e.what();
            return FALSE;
        }
        return TRUE;
    }

    const gchar* getExceptionMessage()
    {
        return exception_message.c_str();
    }

    void analyzeFrame(const GstVideoFrame* frame, gboolean draw_detection)
    {
        auto* pixels = reinterpret_cast<uint8_t*>(GST_VIDEO_FRAME_PLANE_DATA(frame, 0));
        auto stride = GST_VIDEO_FRAME_PLANE_STRIDE(frame, 0);
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

        auto& detection_vec = analyzer.getDetections();
        auto* meta = gst_buffer_add_detection_meta(frame->buffer);
        auto* detection_array = meta->detection_array;
        for (auto& d : detection_vec) {
            auto* detect_struct = gst_structure_new_empty("detection");
            gst_structure_set(detect_struct, "center_x", G_TYPE_FLOAT, d.center_x, NULL);
            gst_structure_set(detect_struct, "center_y", G_TYPE_FLOAT, d.center_y, NULL);
            gst_structure_set(detect_struct, "width", G_TYPE_FLOAT, d.width, NULL);
            gst_structure_set(detect_struct, "height", G_TYPE_FLOAT, d.height, NULL);
            gst_structure_set(detect_struct, "class_id", G_TYPE_INT, d.class_id, NULL);
            gst_structure_set(detect_struct, "confidence", G_TYPE_FLOAT, d.confidence, NULL);
            detection_array = g_array_append_vals(detection_array, detect_struct, 1);
            gst_structure_free(detect_struct);
        }
    }

    const std::vector<detection_t>& getDetections() const
    {
        return analyzer.getDetections();
    }

private:
    VideoAnalyzer analyzer;
    std::string exception_message;
};

#endif
