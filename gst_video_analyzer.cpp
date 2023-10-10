#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include "gst_video_analyzer.h"

GST_DEBUG_CATEGORY_STATIC (video_analyzer_debug);
#define GST_CAT_DEFAULT video_analyzer_debug

enum
{
  PROP_0,
  PROP_DRAW_MARKUP
};

static GstStaticPadTemplate video_analyzer_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ Y444, RGB}"))
    );

static GstStaticPadTemplate video_analyzer_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ Y444, RGB}"))
    );

static void gst_video_analyzer_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec) {
    GstVideoAnalyzer *video_analyzer = GST_VIDEO_ANALYZER (object);

    switch (prop_id) {
        case PROP_DRAW_MARKUP:
            GST_OBJECT_LOCK (video_analyzer);
            video_analyzer->markup_drawing_enabled = g_value_get_boolean (value);
            GST_DEBUG_OBJECT (video_analyzer, "Markup drawing: %s", video_analyzer->markup_drawing_enabled ? "enabled" : "disabled");
            GST_OBJECT_UNLOCK (video_analyzer);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void gst_video_analyzer_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec) {
    GstVideoAnalyzer *video_analyzer = GST_VIDEO_ANALYZER (object);
    switch (prop_id) {
        case PROP_DRAW_MARKUP:
            g_value_set_boolean (value, video_analyzer->markup_drawing_enabled);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
        }
}

G_DEFINE_TYPE (GstVideoAnalyzer, gst_video_analyzer, GST_TYPE_VIDEO_FILTER);

static void gst_video_analyzer_class_init (GstVideoAnalyzerClass * g_class) {
    GST_DEBUG_CATEGORY_INIT(video_analyzer_debug, "video_analyzer", 0, "video_analyzer");

    GObjectClass *gobject_class = (GObjectClass *) g_class;
    gobject_class->set_property = gst_video_analyzer_set_property;
    gobject_class->get_property = gst_video_analyzer_get_property;

    const auto property_flags = GParamFlags(GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_DRAW_MARKUP, g_param_spec_boolean ("draw markup", "Draw Markup", "draw markup", TRUE, property_flags));

    GstElementClass *gstelement_class = (GstElementClass *) g_class;
    gst_element_class_add_static_pad_template (gstelement_class, &video_analyzer_sink_template);
    gst_element_class_add_static_pad_template (gstelement_class, &video_analyzer_src_template);

    GstBaseTransformClass *trans_class = (GstBaseTransformClass *) g_class;
    GstVideoFilterClass *vfilter_class = (GstVideoFilterClass *) g_class;
}

static void gst_video_analyzer_init (GstVideoAnalyzer * video_analyzer) {
    ;
}

