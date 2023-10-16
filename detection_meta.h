#ifndef _DETECTION_META_H_
#define _DETECTION_META_H_

#include <gst/gst.h>

typedef struct _DetectionMeta DetectionMeta;

struct _DetectionMeta {
  GstMeta   meta;
  GArray*   detection_array;
};

GType detection_meta_api_get_type (void);
#define DETECTION_META_API_TYPE (detection_meta_api_get_type())

GType detection_meta_api_get_type (void) {
  static volatile GType type;
  static const gchar *tags[] = { "detection", NULL };

  if (g_once_init_enter (&type)) {
    GType _type = gst_meta_api_type_register("DetectionMetaAPI", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

#define gst_buffer_get_detection_meta(b) ((DetectionMeta*)gst_buffer_get_meta((b), DETECTION_META_API_TYPE))

const GstMetaInfo *detection_meta_meta_get_info (void);
#define DETECTION_META_INFO (detection_meta_get_info())

static gboolean detection_meta_init (GstMeta* meta, gpointer params, GstBuffer * buffer) {
  auto* emeta = (DetectionMeta*)meta;
  emeta->detection_array = g_array_new(FALSE, TRUE, sizeof(GstStructure));
  return TRUE;
}

static void detection_meta_free (GstMeta * meta, GstBuffer * buffer) {
  auto* emeta = (DetectionMeta*)meta;
  if (emeta->detection_array != NULL) {
      g_array_free(emeta->detection_array, TRUE);
  }
}

const GstMetaInfo* detection_meta_get_info (void) {
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *mi = gst_meta_register (DETECTION_META_API_TYPE,
        "DetectionMeta",
        sizeof (DetectionMeta),
        detection_meta_init,
        detection_meta_free,
        NULL);
    g_once_init_leave (&meta_info, mi);
  }
  return meta_info;
}

DetectionMeta* gst_buffer_add_detection_meta(GstBuffer *buffer) {
  g_return_val_if_fail (GST_IS_BUFFER (buffer), NULL);

  auto* meta = (DetectionMeta*)gst_buffer_get_meta(buffer, DETECTION_META_API_TYPE);
  if (meta == NULL) {
        meta = (DetectionMeta*)gst_buffer_add_meta(buffer, DETECTION_META_INFO, NULL);
  } else if (meta->detection_array != NULL) {
      g_array_free(meta->detection_array, TRUE);
      meta->detection_array = g_array_new(FALSE, TRUE, sizeof(GstStructure));
  }
  return meta;
}

#endif