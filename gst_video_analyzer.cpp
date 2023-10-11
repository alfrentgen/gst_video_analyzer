#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include "gst_video_analyzer.h"
#include "wrapper.h"

GST_DEBUG_CATEGORY_STATIC(video_analyzer_debug);
#define GST_CAT_DEFAULT video_analyzer_debug

enum
{
  PROP_0,
  PROP_DRAW_MARKUP,
  PROP_MODEL_PATH,
};

static GstStaticPadTemplate video_analyzer_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ RGB}"))
    );

static GstStaticPadTemplate video_analyzer_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ RGB}"))
    );

static void gst_video_analyzer_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec) {
    GstVideoAnalyzer *video_analyzer = GST_VIDEO_ANALYZER (object);

    GST_OBJECT_LOCK (video_analyzer);
    switch (prop_id) {
        case PROP_DRAW_MARKUP:
            video_analyzer->markup_drawing_enabled = g_value_get_boolean(value);
            GST_DEBUG_OBJECT (video_analyzer, "Markup drawing: %s", video_analyzer->markup_drawing_enabled ? "enabled" : "disabled");
            break;
        case PROP_MODEL_PATH: {
            GST_OBJECT_LOCK (video_analyzer);
            auto* model_path = g_value_get_string(value);
            g_string_assign(video_analyzer->model_path, model_path);
            auto* engine = (VideoAnalyzerWrapper*)video_analyzer->engine;            
            if (engine->setModel(model_path) == FALSE) {
                GST_DEBUG_OBJECT(video_analyzer, "Model setting failed due to: %s. Enable bypassing, since filter has no model now.", engine->getExceptionMessage());
                video_analyzer->markup_drawing_enabled = FALSE;
            }
            break;
        }
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
    GST_OBJECT_UNLOCK (video_analyzer);
}

static void gst_video_analyzer_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec) {
    auto* video_analyzer = GST_VIDEO_ANALYZER(object);
    GST_OBJECT_LOCK (video_analyzer);
    switch (prop_id) {
        case PROP_DRAW_MARKUP:
            g_value_set_boolean (value, video_analyzer->markup_drawing_enabled);
            gst_base_transform_set_passthrough(GST_BASE_TRANSFORM_CAST(video_analyzer), !video_analyzer->markup_drawing_enabled);
            break;
        case PROP_MODEL_PATH:
            g_value_set_string(value, video_analyzer->model_path->str);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
        }
    GST_OBJECT_UNLOCK (video_analyzer);
}

void video_analyzer_before_transform(GstBaseTransform *trans, GstBuffer *buffer) {
    //OCV analysis;
}

GstFlowReturn video_analyzer_transform_frame_ip(GstVideoFilter* vfilter, GstVideoFrame *frame) {
    auto* analyzer = GST_VIDEO_ANALYZER(vfilter);
    auto* engine = (VideoAnalyzerWrapper*)analyzer->engine;
    engine->analyzeFrame(frame);
    if (analyzer->markup_drawing_enabled) {
        engine->drawMarkup(frame);
    }
    return GST_FLOW_OK;
}

static void gst_video_analyzer_finalize(GObject* g_object) {
    auto* video_analyzer = GST_VIDEO_ANALYZER(g_object);
    GST_OBJECT_LOCK (video_analyzer);
    delete (VideoAnalyzerWrapper*)video_analyzer->engine;
    g_string_free(video_analyzer->model_path, TRUE);
    GST_OBJECT_UNLOCK (video_analyzer);
}

static void gst_video_analyzer_class_init (GstVideoAnalyzerClass* g_class) {
    GST_DEBUG_CATEGORY_INIT(video_analyzer_debug, "video_analyzer", 0, "video_analyzer");

    auto* gobject_class = G_OBJECT_CLASS(g_class);
    gobject_class->set_property = gst_video_analyzer_set_property;
    gobject_class->get_property = gst_video_analyzer_get_property;

    const auto property_flags = GParamFlags(GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_DRAW_MARKUP, g_param_spec_boolean ("draw markup", "Draw Markup", "draw markup", TRUE, property_flags));
    g_object_class_install_property(gobject_class, PROP_MODEL_PATH, g_param_spec_string ("model path", "Model Path", "model path", "", property_flags));

    auto* gstelement_class = GST_ELEMENT_CLASS(g_class);
    gst_element_class_add_static_pad_template (gstelement_class, &video_analyzer_sink_template);
    gst_element_class_add_static_pad_template (gstelement_class, &video_analyzer_src_template);

    auto* trans_class = GST_BASE_TRANSFORM_CLASS(g_class);
    trans_class->transform_ip_on_passthrough = FALSE;
    trans_class->before_transform = video_analyzer_before_transform;
    
    auto* vfilter_class = GST_VIDEO_FILTER_CLASS(g_class);
    vfilter_class->transform_frame_ip = nullptr;

    gobject_class->finalize = gst_video_analyzer_finalize;
}

static void gst_video_analyzer_init(GstVideoAnalyzer* video_analyzer) {
    video_analyzer->markup_drawing_enabled = TRUE;
    video_analyzer->model_path = g_string_new("");
    video_analyzer->engine = new VideoAnalyzerWrapper();
}

G_DEFINE_TYPE (GstVideoAnalyzer, gst_video_analyzer, GST_TYPE_VIDEO_FILTER);