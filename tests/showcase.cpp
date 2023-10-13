#include <gst/gst.h>
#include <glib.h>

static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}


static void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  auto* src_pad_name = gst_pad_get_name(pad);
  if (strncmp(src_pad_name, "video", 5) != 0) {
    return;
  }

  GstElement* downstream = (GstElement*)data;

  /* We can now link this pad with the downstream sink pad */
  g_print ("Dynamic pad created, linking demuxer to video parser.\n");

  auto* sinkpad = gst_element_get_static_pad (downstream, "sink");
  auto* peer = gst_pad_get_peer(sinkpad);
  if (peer == nullptr) {
    auto ret = gst_pad_link (pad, sinkpad);
    if (ret != GstPadLinkReturn::GST_PAD_LINK_OK) {
          g_print ("Could not link demuxer!\n");

    }
  }
  
  if(peer)
    gst_object_unref(peer);
  gst_object_unref(sinkpad);
}



int
main (int   argc,
      char *argv[])
{
  GMainLoop *loop;

  //GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
  GstBus *bus;
  guint bus_watch_id;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);


  /* Check input arguments */
  if (argc != 2) {
    g_printerr ("Usage: %s <Video filename>\n", argv[0]);
    return -1;
  }


  /* Create gstreamer elements */
  auto* pipeline = gst_pipeline_new ("audio-player");
  auto* source   = gst_element_factory_make ("filesrc",               "source");
  auto* demuxer  = gst_element_factory_make ("qtdemux",               "demuxer");
  auto* h264parse = gst_element_factory_make("h264parse",             "videoparser");
  auto* decoder  = gst_element_factory_make ("avdec_h264",            "decoder");
  auto* conv     = gst_element_factory_make ("videoconvert",          "converter");
  auto* analyzer = gst_element_factory_make ("video_analyzer",        "analyzer");
  auto* conv2     = gst_element_factory_make ("videoconvert",         "converter2");
  auto* sink     = gst_element_factory_make ("autovideosink",         "sink");

  if (!pipeline || !source || !demuxer || !decoder || !conv || !analyzer || !conv2 || !sink) {
    if (!analyzer) {
      g_printerr ("No analyzer!\n");
    }
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */

  /* we set the input filename to the source element */
  g_object_set (G_OBJECT (source), "location", argv[1], NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, demuxer, h264parse, decoder, conv, analyzer, conv2, sink, NULL);

  /* we link the elements together */
  g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), h264parse);
  gst_element_link (source, demuxer);
  gst_element_link_many (h264parse, decoder, conv, analyzer, conv2, sink, NULL);

  /* Set the pipeline to "playing" state*/
  g_print ("Now playing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  return 0;
}
