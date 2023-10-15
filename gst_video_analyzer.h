#ifndef _GST_VIDEO_ANALYZER_H_
#define _GST_VIDEO_ANALYZER_H_

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define GST_TYPE_VIDEO_ANALYZER   (gst_video_analyzer_get_type())
#define GST_VIDEO_ANALYZER(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_VIDEO_ANALYZER, GstVideoAnalyzer))
#define GST_VIDEO_ANALYZER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_VIDEO_ANALYZER, GstVideoAnalyzerClass))
#define GST_IS_VIDEO_ANALYZER(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_VIDEO_ANALYZER))
#define GST_IS_VIDEO_ANALYZER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_VIDEO_ANALYZER))

typedef struct _GstVideoAnalyzer GstVideoAnalyzer;
typedef struct _GstVideoAnalyzerClass GstVideoAnalyzerClass;

struct _GstVideoAnalyzer
{
    GstVideoFilter base_video_filter;
    GstPad * sinkpad;
    GstPad * srcpad;
    
    gboolean draw_detections;
    GString* model_path;

    void* engine;

};

struct _GstVideoAnalyzerClass
{
  GstVideoFilterClass base_video_filter_class;
};

GType gst_video_analyzer_get_type (void);

G_END_DECLS

#endif