/*
 * Cogl
 *
 * A Low Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2007, 2008 OpenedHand
 * Copyright (C) 2009, 2010, 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "config.h"

#include <gst/gst.h>
#include <gst/gstvalue.h>
#include <gst/video/video.h>
#include <gst/riff/riff-ids.h>
#include <string.h>
#include <math.h>

#include "cogl-gtype-private.h"

/* We just need the public Cogl api for cogl-gst but we first need to
 * undef COGL_COMPILATION to avoid getting an error that normally
 * checks cogl.h isn't used internally. */
#undef COGL_COMPILATION
#include <cogl/cogl.h>

#include "cogl-gst-video-sink.h"

#define COGL_GST_DEFAULT_PRIORITY G_PRIORITY_HIGH_IDLE

#define BASE_SINK_CAPS "{ AYUV," \
                       "YV12," \
                       "I420," \
                       "RGBA," \
                       "BGRA," \
                       "RGB," \
                       "BGR," \
                       "NV12 }"

#define COGL_GST_PARAM_STATIC        \
  (G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB)

#define COGL_GST_PARAM_READABLE      \
  (G_PARAM_READABLE | COGL_GST_PARAM_STATIC)

#define COGL_GST_PARAM_WRITABLE      \
  (G_PARAM_WRITABLE | COGL_GST_PARAM_STATIC)

#define COGL_GST_PARAM_READWRITE     \
  (G_PARAM_READABLE | G_PARAM_WRITABLE | COGL_GST_PARAM_STATIC)

static const char cogl_gst_video_sink_caps_str[] =
  GST_VIDEO_CAPS_MAKE_WITH_FEATURES(GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY, BASE_SINK_CAPS);

static GstStaticPadTemplate sinktemplate_all =
  GST_STATIC_PAD_TEMPLATE ("sink",
                           GST_PAD_SINK,
                           GST_PAD_ALWAYS,
                           GST_STATIC_CAPS (cogl_gst_video_sink_caps_str));

static void color_balance_iface_init (GstColorBalanceInterface *iface);

G_DEFINE_TYPE_WITH_CODE (CoglGstVideoSink,
                         cogl_gst_video_sink,
                         GST_TYPE_BASE_SINK,
                         G_IMPLEMENT_INTERFACE (GST_TYPE_COLOR_BALANCE, color_balance_iface_init))

enum
{
  PROP_0,
  PROP_UPDATE_PRIORITY
};

enum
{
  PIPELINE_READY_SIGNAL,
  NEW_FRAME_SIGNAL,

  LAST_SIGNAL
};

static guint video_sink_signals[LAST_SIGNAL] = { 0, };

typedef enum
{
  COGL_GST_NOFORMAT,
  COGL_GST_RGB32,
  COGL_GST_RGB24,
  COGL_GST_AYUV,
  COGL_GST_YV12,
  COGL_GST_SURFACE,
  COGL_GST_I420,
  COGL_GST_NV12
} CoglGstVideoFormat;

typedef enum
{
  COGL_GST_RENDERER_NEEDS_GLSL = (1 << 0),
  COGL_GST_RENDERER_NEEDS_TEXTURE_RG = (1 << 1)
} CoglGstRendererFlag;

/* We want to cache the snippets instead of recreating a new one every
 * time we initialise a pipeline so that if we end up recreating the
 * same pipeline again then Cogl will be able to use the pipeline
 * cache to avoid linking a redundant identical shader program */
typedef struct
{
  CoglSnippet *vertex_snippet;
  CoglSnippet *fragment_snippet;
  CoglSnippet *default_sample_snippet;
  int start_position;
} SnippetCacheEntry;

typedef struct
{
  GQueue entries;
} SnippetCache;

typedef struct _CoglGstSource
{
  GSource source;
  CoglGstVideoSink *sink;
  GMutex buffer_lock;
  GstBuffer *buffer;
  CoglBool has_new_caps;
} CoglGstSource;

typedef void (CoglGstRendererPaint) (CoglGstVideoSink *);
typedef void (CoglGstRendererPostPaint) (CoglGstVideoSink *);

typedef struct _CoglGstRenderer
{
  const char *name;
  CoglGstVideoFormat format;
  int flags;
  GstStaticCaps caps;
  int n_layers;
  void (*setup_pipeline) (CoglGstVideoSink *sink,
                          CoglPipeline *pipeline);
  CoglBool (*upload) (CoglGstVideoSink *sink,
                      GstBuffer *buffer);
  void (*shutdown) (CoglGstVideoSink *sink);
} CoglGstRenderer;

struct _CoglGstVideoSinkPrivate
{
  CoglContext *ctx;
  CoglPipeline *pipeline;
  CoglTexture *frame[3];
  CoglBool frame_dirty;
  CoglBool had_upload_once;

  CoglGstVideoFormat format;
  CoglBool bgr;

  CoglGstSource *source;
  GSList *renderers;
  GstCaps *caps;
  CoglGstRenderer *renderer;
  GstFlowReturn flow_return;
  int custom_start;
  int video_start;
  int free_layer;
  CoglBool default_sample;
  GstVideoInfo info;

  gdouble brightness;
  gdouble contrast;
  gdouble hue;
  gdouble saturation;
  CoglBool balance_dirty;

  guint8 *tabley;
  guint8 *tableu;
  guint8 *tablev;
};

/* GTypes */

static gpointer
cogl_gst_rectangle_copy (gpointer src)
{
  if (G_LIKELY (src))
    {
      CoglGstRectangle *new = g_slice_new (CoglGstRectangle);
      memcpy (new, src, sizeof (CoglGstRectangle));
      return new;
    }
  else
    return NULL;
}

static void
cogl_gst_rectangle_free (gpointer ptr)
{
  g_slice_free (CoglGstRectangle, ptr);
}

COGL_GTYPE_DEFINE_BOXED (GstRectangle,
                         gst_rectangle,
                         cogl_gst_rectangle_copy,
                         cogl_gst_rectangle_free);

/* Snippet cache */

static SnippetCacheEntry *
get_layer_cache_entry (CoglGstVideoSink *sink,
                       SnippetCache *cache)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  GList *l;

  for (l = cache->entries.head; l; l = l->next)
    {
      SnippetCacheEntry *entry = l->data;

      if (entry->start_position == priv->video_start)
        return entry;
    }

  return NULL;
}

static SnippetCacheEntry *
add_layer_cache_entry (CoglGstVideoSink *sink,
                       SnippetCache *cache,
                       const char *decl)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  SnippetCacheEntry *entry = g_slice_new (SnippetCacheEntry);
  char *default_source;

  entry->start_position = priv->video_start;

  entry->vertex_snippet =
    cogl_snippet_new (COGL_SNIPPET_HOOK_VERTEX_GLOBALS,
                      decl,
                      NULL /* post */);
  entry->fragment_snippet =
    cogl_snippet_new (COGL_SNIPPET_HOOK_FRAGMENT_GLOBALS,
                      decl,
                      NULL /* post */);

  default_source =
    g_strdup_printf ("  cogl_layer *= cogl_gst_sample_video%i "
                     "(cogl_tex_coord%i_in.st);\n",
                     priv->video_start,
                     priv->video_start);
  entry->default_sample_snippet =
    cogl_snippet_new (COGL_SNIPPET_HOOK_LAYER_FRAGMENT,
                      NULL, /* declarations */
                      default_source);
  g_free (default_source);

  g_queue_push_head (&cache->entries, entry);

  return entry;
}

static SnippetCacheEntry *
get_global_cache_entry (SnippetCache *cache, int param)
{
  GList *l;

  for (l = cache->entries.head; l; l = l->next)
    {
      SnippetCacheEntry *entry = l->data;

      if (entry->start_position == param)
        return entry;
    }

  return NULL;
}

static SnippetCacheEntry *
add_global_cache_entry (SnippetCache *cache,
                        const char *decl,
                        int param)
{
  SnippetCacheEntry *entry = g_slice_new (SnippetCacheEntry);

  entry->start_position = param;

  entry->vertex_snippet =
    cogl_snippet_new (COGL_SNIPPET_HOOK_VERTEX_GLOBALS,
                      decl,
                      NULL /* post */);
  entry->fragment_snippet =
    cogl_snippet_new (COGL_SNIPPET_HOOK_FRAGMENT_GLOBALS,
                      decl,
                      NULL /* post */);

  g_queue_push_head (&cache->entries, entry);

  return entry;
}

static void
setup_pipeline_from_cache_entry (CoglGstVideoSink *sink,
                                 CoglPipeline *pipeline,
                                 SnippetCacheEntry *cache_entry,
                                 int n_layers)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;

  if (cache_entry)
    {
      int i;

      /* The global sampling function gets added to both the fragment
       * and vertex stages. The hope is that the GLSL compiler will
       * easily remove the dead code if it's not actually used */
      cogl_pipeline_add_snippet (pipeline, cache_entry->vertex_snippet);
      cogl_pipeline_add_snippet (pipeline, cache_entry->fragment_snippet);

      /* Set all of the layers to just directly copy from the previous
       * layer so that it won't redundantly generate code to sample
       * the intermediate textures */
      for (i = 0; i < n_layers; i++) {
        cogl_pipeline_set_layer_combine (pipeline,
                                         priv->video_start + i,
                                         "RGBA=REPLACE(PREVIOUS)",
                                         NULL);
      }

      if (priv->default_sample) {
        cogl_pipeline_add_layer_snippet (pipeline,
                                         priv->video_start + n_layers - 1,
                                         cache_entry->default_sample_snippet);
      }
    }

  priv->frame_dirty = TRUE;
}

/* Color balance */

#define DEFAULT_BRIGHTNESS (0.0f)
#define DEFAULT_CONTRAST   (1.0f)
#define DEFAULT_HUE        (0.0f)
#define DEFAULT_SATURATION (1.0f)

static CoglBool
cogl_gst_video_sink_needs_color_balance_shader (CoglGstVideoSink *sink)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;

  return (priv->brightness != DEFAULT_BRIGHTNESS ||
          priv->contrast != DEFAULT_CONTRAST ||
          priv->hue != DEFAULT_HUE ||
          priv->saturation != DEFAULT_SATURATION);
}

static void
cogl_gst_video_sink_color_balance_update_tables (CoglGstVideoSink *sink)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  gint i, j;
  gdouble y, u, v, hue_cos, hue_sin;

  /* Y */
  for (i = 0; i < 256; i++) {
    y = 16 + ((i - 16) * priv->contrast + priv->brightness * 255);
    if (y < 0)
      y = 0;
    else if (y > 255)
      y = 255;
    priv->tabley[i] = rint (y);
  }

  hue_cos = cos (G_PI * priv->hue);
  hue_sin = sin (G_PI * priv->hue);

  /* U/V lookup tables are 2D, since we need both U/V for each table
   * separately. */
  for (i = -128; i < 128; i++) {
    for (j = -128; j < 128; j++) {
      u = 128 + ((i * hue_cos + j * hue_sin) * priv->saturation);
      v = 128 + ((-i * hue_sin + j * hue_cos) * priv->saturation);
      if (u < 0)
        u = 0;
      else if (u > 255)
        u = 255;
      if (v < 0)
        v = 0;
      else if (v > 255)
        v = 255;
      priv->tableu[(i + 128) * 256 + j + 128] = rint (u);
      priv->tablev[(i + 128) * 256 + j + 128] = rint (v);
    }
  }
}

static const GList *
cogl_gst_video_sink_color_balance_list_channels (GstColorBalance *balance)
{
  static GList *channels = NULL;

  if (channels == NULL) {
    const gchar *str_channels[4] = { "HUE", "SATURATION",
                                     "BRIGHTNESS", "CONTRAST"
    };
    gint i;

    for (i = 0; i < G_N_ELEMENTS (str_channels); i++) {
      GstColorBalanceChannel *channel;

      channel = g_object_new (GST_TYPE_COLOR_BALANCE_CHANNEL, NULL);
      channel->label = g_strdup (str_channels[i]);
      channel->min_value = -1000;
      channel->max_value = 1000;
      channels = g_list_append (channels, channel);
    }
  }

  return channels;
}

static gboolean
cogl_gst_video_sink_get_variable (CoglGstVideoSink *sink,
                                  const gchar *variable,
                                  gdouble *minp,
                                  gdouble *maxp,
                                  gdouble **valuep)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  gdouble min, max, *value;

  if (!g_strcmp0 (variable, "BRIGHTNESS"))
    {
      min = -1.0;
      max = 1.0;
      value = &priv->brightness;
    }
  else if (!g_strcmp0 (variable, "CONTRAST"))
    {
      min = 0.0;
      max = 2.0;
      value = &priv->contrast;
    }
  else if (!g_strcmp0 (variable, "HUE"))
    {
      min = -1.0;
      max = 1.0;
      value = &priv->hue;
    }
  else if (!g_strcmp0 (variable, "SATURATION"))
    {
      min = 0.0;
      max = 2.0;
      value = &priv->saturation;
    }
  else
    {
      GST_WARNING_OBJECT (sink, "color balance parameter not supported %s",
                          variable);
      return FALSE;
    }

  if (maxp)
    *maxp = max;
  if (minp)
    *minp = min;
  if (valuep)
    *valuep = value;

  return TRUE;
}

static void
cogl_gst_video_sink_color_balance_set_value (GstColorBalance        *balance,
                                             GstColorBalanceChannel *channel,
                                             gint                    value)
{
  CoglGstVideoSink *sink = COGL_GST_VIDEO_SINK (balance);
  gdouble *old_value, new_value, min, max;

  if (!cogl_gst_video_sink_get_variable (sink, channel->label,
                                         &min, &max, &old_value))
    return;

  new_value = (max - min) * ((gdouble) (value - channel->min_value) /
                             (gdouble) (channel->max_value - channel->min_value))
    + min;

  if (new_value != *old_value)
    {
      *old_value = new_value;
      sink->priv->balance_dirty = TRUE;

      gst_color_balance_value_changed (GST_COLOR_BALANCE (balance), channel,
                                       gst_color_balance_get_value (GST_COLOR_BALANCE (balance), channel));
    }
}

static gint
cogl_gst_video_sink_color_balance_get_value (GstColorBalance        *balance,
                                             GstColorBalanceChannel *channel)
{
  CoglGstVideoSink *sink = COGL_GST_VIDEO_SINK (balance);
  gdouble *old_value, min, max;
  gint value;

  if (!cogl_gst_video_sink_get_variable (sink, channel->label,
                                         &min, &max, &old_value))
    return 0;

  value = (gint) (((*old_value + min) / (max - min)) *
                  (channel->max_value - channel->min_value))
    + channel->min_value;

  return value;
}

static GstColorBalanceType
cogl_gst_video_sink_color_balance_get_balance_type (GstColorBalance *balance)
{
  return GST_COLOR_BALANCE_HARDWARE;
}

static void
color_balance_iface_init (GstColorBalanceInterface *iface)
{
  iface->list_channels = cogl_gst_video_sink_color_balance_list_channels;
  iface->set_value = cogl_gst_video_sink_color_balance_set_value;
  iface->get_value = cogl_gst_video_sink_color_balance_get_value;

  iface->get_balance_type = cogl_gst_video_sink_color_balance_get_balance_type;
}

/**/

static void
cogl_gst_source_finalize (GSource *source)
{
  CoglGstSource *gst_source = (CoglGstSource *) source;

  g_mutex_lock (&gst_source->buffer_lock);
  if (gst_source->buffer)
    gst_buffer_unref (gst_source->buffer);
  gst_source->buffer = NULL;
  g_mutex_unlock (&gst_source->buffer_lock);
  g_mutex_clear (&gst_source->buffer_lock);
}

int
cogl_gst_video_sink_get_free_layer (CoglGstVideoSink *sink)
{
  return sink->priv->free_layer;
}

void
cogl_gst_video_sink_attach_frame (CoglGstVideoSink *sink,
                                  CoglPipeline *pln)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  int i;

  for (i = 0; i < G_N_ELEMENTS (priv->frame); i++)
    if (priv->frame[i] != NULL)
      cogl_pipeline_set_layer_texture (pln, i + priv->video_start,
                                       priv->frame[i]);
}

/* Color balance */

static const gchar *no_color_balance_shader =
  "#define cogl_gst_get_corrected_color_from_yuv(arg) (arg)\n"
  "#define cogl_gst_get_corrected_color_from_rgb(arg) (arg)\n";

static const gchar *color_balance_shader =
  "vec3\n"
  "cogl_gst_get_corrected_color_from_yuv (vec3 yuv)\n"
  "{\n"
  "  vec2 ruv = vec2 (yuv[2] + 0.5, yuv[1] + 0.5);\n"
  "  return vec3 (texture2D (cogl_sampler%i, vec2 (yuv[0], 0)).a,\n"
  "               texture2D (cogl_sampler%i, ruv).a - 0.5,\n"
  "               texture2D (cogl_sampler%i, ruv).a - 0.5);\n"
  "}\n"
  "\n"
  "vec3\n"
  "cogl_gst_get_corrected_color_from_rgb (vec3 rgb)\n"
  "{\n"
  "  vec3 yuv = cogl_gst_yuv_srgb_to_bt601 (rgb);\n"
  "  vec3 corrected_yuv = vec3 (texture2D (cogl_sampler%i, vec2 (yuv[0], 0)).a,\n"
  "                             texture2D (cogl_sampler%i, vec2 (yuv[2], yuv[1])).a,\n"
  "                             texture2D (cogl_sampler%i, vec2 (yuv[2], yuv[1])).a);\n"
  "  return cogl_gst_yuv_bt601_to_srgb (corrected_yuv);\n"
  "}\n";

static void
cogl_gst_video_sink_setup_balance (CoglGstVideoSink *sink,
                                   CoglPipeline *pipeline)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  static SnippetCache snippet_cache;
  static CoglSnippet *no_color_balance_snippet_vert = NULL,
    *no_color_balance_snippet_frag = NULL;

  GST_INFO_OBJECT (sink, "attaching correction b=%.3f/c=%.3f/h=%.3f/s=%.3f",
                   priv->brightness, priv->contrast,
                   priv->hue, priv->saturation);

  if (cogl_gst_video_sink_needs_color_balance_shader (sink))
    {
      int i;
      const guint8 *tables[3] = { priv->tabley, priv->tableu, priv->tablev };
      const gint tables_sizes[3][2] = { { 256, 1 },
                                        { 256, 256 },
                                        { 256, 256 } };
      SnippetCacheEntry *entry = get_layer_cache_entry (sink, &snippet_cache);

      if (entry == NULL)
        {
          gchar *source = g_strdup_printf (color_balance_shader,
                                           priv->custom_start,
                                           priv->custom_start + 1,
                                           priv->custom_start + 2,
                                           priv->custom_start,
                                           priv->custom_start + 1,
                                           priv->custom_start + 2);

          entry = add_layer_cache_entry (sink, &snippet_cache, source);
          g_free (source);
        }

      cogl_pipeline_add_snippet (pipeline, entry->vertex_snippet);
      cogl_pipeline_add_snippet (pipeline, entry->fragment_snippet);

      cogl_gst_video_sink_color_balance_update_tables (sink);

      for (i = 0; i < 3; i++)
        {
          CoglTexture *lut_texture =
            cogl_texture_2d_new_from_data (priv->ctx,
                                           tables_sizes[i][0],
                                           tables_sizes[i][1],
                                           COGL_PIXEL_FORMAT_A_8,
                                           tables_sizes[i][0],
                                           tables[i],
                                           NULL);

          cogl_pipeline_set_layer_filters (pipeline,
                                           priv->custom_start + i,
                                           COGL_PIPELINE_FILTER_LINEAR,
                                           COGL_PIPELINE_FILTER_LINEAR);
          cogl_pipeline_set_layer_combine (pipeline,
                                           priv->custom_start + i,
                                           "RGBA=REPLACE(PREVIOUS)",
                                           NULL);
          cogl_pipeline_set_layer_texture (pipeline,
                                           priv->custom_start + i,
                                           lut_texture);

          cogl_object_unref (lut_texture);
        }

      priv->video_start = priv->custom_start + 3;
    }
  else
    {
      if (G_UNLIKELY (no_color_balance_snippet_vert == NULL))
        {
          no_color_balance_snippet_vert =
            cogl_snippet_new (COGL_SNIPPET_HOOK_VERTEX_GLOBALS,
                              no_color_balance_shader,
                              NULL);
          no_color_balance_snippet_frag =
            cogl_snippet_new (COGL_SNIPPET_HOOK_FRAGMENT_GLOBALS,
                              no_color_balance_shader,
                              NULL);
        }

      cogl_pipeline_add_snippet (pipeline, no_color_balance_snippet_vert);
      cogl_pipeline_add_snippet (pipeline, no_color_balance_snippet_frag);

      priv->video_start = priv->custom_start;
    }
}

/* YUV <-> RGB conversions */

static const gchar *color_conversions_shaders =
  "\n"
  "/* These conversion functions take : */\n"
  "/*   Y = [0, 1] */\n"
  "/*   U = [-0.5, 0.5] */\n"
  "/*   V = [-0.5, 0.5] */\n"
  "vec3\n"
  "cogl_gst_yuv_bt601_to_srgb (vec3 yuv)\n"
  "{\n"
  "  return mat3 (1.0,    1.0,      1.0,\n"
  "               0.0,   -0.344136, 1.772,\n"
  "               1.402, -0.714136, 0.0   ) * yuv;\n"
  "}\n"
  "\n"
  "vec3\n"
  "cogl_gst_yuv_bt709_to_srgb (vec3 yuv)\n"
  "{\n"
  "  return mat3 (1.0,     1.0,      1.0,\n"
  "               0.0,    -0.187324, 1.8556,\n"
  "               1.5748, -0.468124, 0.0    ) * yuv;\n"
  "}\n"
  "\n"
  "vec3\n"
  "cogl_gst_yuv_bt2020_to_srgb (vec3 yuv)\n"
  "{\n"
  "  return mat3 (1.0,     1.0,      1.0,\n"
  "               0.0,     0.571353, 1.8814,\n"
  "               1.4746,  0.164553, 0.0    ) * yuv;\n"
  "}\n"
  "/* Original transformation, still no idea where these values come from... */\n"
  "vec3\n"
  "cogl_gst_yuv_originalyuv_to_srgb (vec3 yuv)\n"
  "{\n"
  "  return mat3 (1.0,         1.0,      1.0,\n"
  "               0.0,        -0.390625, 2.015625,\n"
  "               1.59765625, -0.8125,   0.0      ) * yuv;\n"
  "}\n"
  "\n"
  "vec3\n"
  "cogl_gst_yuv_srgb_to_bt601 (vec3 rgb)\n"
  "{\n"
  "  return mat3 (0.299,  0.5,      -0.168736,\n"
  "               0.587, -0.418688, -0.331264,\n"
  "               0.114, -0.081312,  0.5      ) * rgb;\n"
  "}\n"
  "\n"
  "vec3\n"
  "cogl_gst_yuv_srgb_to_bt709 (vec3 rgb)\n"
  "{\n"
  "  return mat3 (0.2126, -0.114626,  0.5,\n"
  "               0.7152, -0.385428, -0.454153,\n"
  "               0.0722,  0.5,       0.045847 ) * rgb;\n"
  "}\n"
  "\n"
  "vec3\n"
  "cogl_gst_yuv_srgb_to_bt2020 (vec3 rgb)\n"
  "{\n"
  "  return mat3 (0.2627, -0.139630,  0.503380,\n"
  "               0.6780, -0.360370, -0.462893,\n"
  "               0.0593,  0.5,      -0.040486 ) * rgb;\n"
  "}\n"
  "\n"
  "#define cogl_gst_default_yuv_to_srgb(arg) cogl_gst_yuv_%s_to_srgb(arg)\n"
  "\n";

static const char *
_gst_video_color_matrix_to_string (GstVideoColorMatrix matrix)
{
  switch (matrix)
    {
    case GST_VIDEO_COLOR_MATRIX_BT601:
      return "bt601";
    case GST_VIDEO_COLOR_MATRIX_BT709:
      return "bt709";

    default:
      return "bt709";
    }
}

static void
cogl_gst_video_sink_setup_conversions (CoglGstVideoSink *sink,
                                       CoglPipeline *pipeline)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  GstVideoColorMatrix matrix = priv->info.colorimetry.matrix;
  static SnippetCache snippet_cache;
  SnippetCacheEntry *entry = get_global_cache_entry (&snippet_cache, matrix);

  if (entry == NULL)
    {
      char *source = g_strdup_printf (color_conversions_shaders,
                                      _gst_video_color_matrix_to_string (matrix));

      entry = add_global_cache_entry (&snippet_cache, source, matrix);
      g_free (source);
    }

  cogl_pipeline_add_snippet (pipeline, entry->vertex_snippet);
  cogl_pipeline_add_snippet (pipeline, entry->fragment_snippet);
}

/**/

static CoglBool
cogl_gst_source_prepare (GSource *source,
                         int *timeout)
{
  CoglGstSource *gst_source = (CoglGstSource *) source;

  *timeout = -1;

  return gst_source->buffer != NULL;
}

static CoglBool
cogl_gst_source_check (GSource *source)
{
  CoglGstSource *gst_source = (CoglGstSource *) source;

  return (gst_source->buffer != NULL ||
          gst_source->sink->priv->balance_dirty);
}

static void
cogl_gst_video_sink_set_priority (CoglGstVideoSink *sink,
                                  int priority)
{
  if (sink->priv->source)
    g_source_set_priority ((GSource *) sink->priv->source, priority);
}

static void
dirty_default_pipeline (CoglGstVideoSink *sink)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;

  if (priv->pipeline)
    {
      cogl_object_unref (priv->pipeline);
      priv->pipeline = NULL;
      priv->had_upload_once = FALSE;
    }
}

static int
_cogl_gst_video_sink_get_video_layer (CoglGstVideoSink *sink)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;

  if (cogl_gst_video_sink_needs_color_balance_shader (sink))
    return priv->custom_start + 3;
  return priv->custom_start;
}

static int
_cogl_gst_video_sink_get_free_layer (CoglGstVideoSink *sink)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  int video_layer = _cogl_gst_video_sink_get_video_layer (sink);

  if (priv->renderer)
    return video_layer + priv->renderer->n_layers;
  return video_layer;
}

void
cogl_gst_video_sink_set_first_layer (CoglGstVideoSink *sink,
                                     int first_layer)
{
  g_return_if_fail (COGL_GST_IS_VIDEO_SINK (sink));

  if (first_layer != sink->priv->custom_start)
    {
      sink->priv->custom_start = first_layer;
      dirty_default_pipeline (sink);

      sink->priv->free_layer = _cogl_gst_video_sink_get_free_layer (sink);
    }
}

void
cogl_gst_video_sink_set_default_sample (CoglGstVideoSink *sink,
                                        CoglBool default_sample)
{
  g_return_if_fail (COGL_GST_IS_VIDEO_SINK (sink));

  if (default_sample != sink->priv->default_sample)
    {
      sink->priv->default_sample = default_sample;
      dirty_default_pipeline (sink);
    }
}

void
cogl_gst_video_sink_setup_pipeline (CoglGstVideoSink *sink,
                                    CoglPipeline *pipeline)
{
  g_return_if_fail (COGL_GST_IS_VIDEO_SINK (sink));

  if (sink->priv->renderer)
    {
      cogl_gst_video_sink_setup_conversions (sink, pipeline);
      cogl_gst_video_sink_setup_balance (sink, pipeline);
      sink->priv->renderer->setup_pipeline (sink, pipeline);
    }
}

CoglPipeline *
cogl_gst_video_sink_get_pipeline (CoglGstVideoSink *vt)
{
  CoglGstVideoSinkPrivate *priv;

  g_return_val_if_fail (COGL_GST_IS_VIDEO_SINK (vt), NULL);

  priv = vt->priv;

  if (priv->pipeline == NULL)
    {
      priv->pipeline = cogl_pipeline_new (priv->ctx);
      cogl_gst_video_sink_setup_pipeline (vt, priv->pipeline);
      cogl_gst_video_sink_attach_frame (vt, priv->pipeline);
      priv->balance_dirty = FALSE;
    }
  else if (priv->balance_dirty)
    {
      cogl_object_unref (priv->pipeline);
      priv->pipeline = cogl_pipeline_new (priv->ctx);

      cogl_gst_video_sink_setup_pipeline (vt, priv->pipeline);
      cogl_gst_video_sink_attach_frame (vt, priv->pipeline);
      priv->balance_dirty = FALSE;
    }
  else if (priv->frame_dirty)
    {
      CoglPipeline *pipeline = cogl_pipeline_copy (priv->pipeline);
      cogl_object_unref (priv->pipeline);
      priv->pipeline = pipeline;

      cogl_gst_video_sink_attach_frame (vt, pipeline);
    }

  priv->frame_dirty = FALSE;

  return priv->pipeline;
}

static void
clear_frame_textures (CoglGstVideoSink *sink)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  int i;

  for (i = 0; i < G_N_ELEMENTS (priv->frame); i++)
    {
      if (priv->frame[i] == NULL)
        break;
      else
        cogl_object_unref (priv->frame[i]);
    }

  memset (priv->frame, 0, sizeof (priv->frame));

  priv->frame_dirty = TRUE;
}

/**/

static void
cogl_gst_dummy_shutdown (CoglGstVideoSink *sink)
{
}

/**/

static inline CoglBool
is_pot (unsigned int number)
{
  /* Make sure there is only one bit set */
  return (number & (number - 1)) == 0;
}

/* This first tries to upload the texture to a CoglTexture2D, but
 * if that's not possible it falls back to a CoglTexture2DSliced.
 *
 * Auto-mipmapping of any uploaded texture is disabled
 */
static CoglTexture *
video_texture_new_from_data (CoglContext *ctx,
                             int width,
                             int height,
                             CoglPixelFormat format,
                             int rowstride,
                             const uint8_t *data)
{
  CoglBitmap *bitmap;
  CoglTexture *tex;
  CoglError *internal_error = NULL;

  bitmap = cogl_bitmap_new_for_data (ctx,
                                     width, height,
                                     format,
                                     rowstride,
                                     (uint8_t *) data);

  if ((is_pot (cogl_bitmap_get_width (bitmap)) &&
       is_pot (cogl_bitmap_get_height (bitmap))) ||
      cogl_has_feature (ctx, COGL_FEATURE_ID_TEXTURE_NPOT_BASIC))
    {
      tex = cogl_texture_2d_new_from_bitmap (bitmap);

      cogl_texture_set_premultiplied (tex, FALSE);

      if (!cogl_texture_allocate (tex, &internal_error))
        {
          cogl_error_free (internal_error);
          internal_error = NULL;
          cogl_object_unref (tex);
          tex = NULL;
        }
    }
  else
    tex = NULL;

  if (!tex)
    {
      /* Otherwise create a sliced texture */
      tex = cogl_texture_2d_sliced_new_from_bitmap (bitmap,
                                                    -1); /* no maximum waste */

      cogl_texture_set_premultiplied (tex, FALSE);

      cogl_texture_allocate (tex, NULL);
    }

  cogl_object_unref (bitmap);

  return tex;
}

static void
cogl_gst_rgb24_glsl_setup_pipeline (CoglGstVideoSink *sink,
                                    CoglPipeline *pipeline)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  static SnippetCache snippet_cache;
  SnippetCacheEntry *entry = get_layer_cache_entry (sink, &snippet_cache);

  if (entry == NULL)
    {
      char *source;

      source =
        g_strdup_printf ("vec4\n"
                         "cogl_gst_sample_video%i (vec2 UV)\n"
                         "{\n"
                         "  vec4 color = texture2D (cogl_sampler%i, UV);\n"
                         "  vec3 corrected = cogl_gst_get_corrected_color_from_rgb (color.rgb);\n"
                         "  return vec4(corrected.rgb, color.a);\n"
                         "}\n",
                         priv->custom_start,
                         priv->custom_start);

      entry = add_layer_cache_entry (sink, &snippet_cache, source);
      g_free (source);
    }

  setup_pipeline_from_cache_entry (sink, pipeline, entry, 1);
}

static void
cogl_gst_rgb24_setup_pipeline (CoglGstVideoSink *sink,
                               CoglPipeline *pipeline)
{
  setup_pipeline_from_cache_entry (sink, pipeline, NULL, 1);
}

static CoglBool
cogl_gst_rgb24_upload (CoglGstVideoSink *sink,
                       GstBuffer *buffer)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  CoglPixelFormat format;
  GstVideoFrame frame;

  if (priv->bgr)
    format = COGL_PIXEL_FORMAT_BGR_888;
  else
    format = COGL_PIXEL_FORMAT_RGB_888;

  if (!gst_video_frame_map (&frame, &priv->info, buffer, GST_MAP_READ))
    goto map_fail;

  clear_frame_textures (sink);

  priv->frame[0] = video_texture_new_from_data (priv->ctx,
                                                GST_VIDEO_FRAME_COMP_WIDTH (&frame, 0),
                                                GST_VIDEO_FRAME_COMP_HEIGHT (&frame, 0),
                                                format,
                                                GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 0),
                                                GST_VIDEO_FRAME_PLANE_DATA (&frame, 0));

  gst_video_frame_unmap (&frame);

  return TRUE;

map_fail:
  {
    GST_ERROR_OBJECT (sink, "Could not map incoming video frame");
    return FALSE;
  }
}

static CoglGstRenderer rgb24_glsl_renderer =
{
  "RGB 24",
  COGL_GST_RGB24,
  COGL_GST_RENDERER_NEEDS_GLSL,

  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES (GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY,
                                                      "{ RGB, BGR }")),
  1, /* n_layers */
  cogl_gst_rgb24_glsl_setup_pipeline,
  cogl_gst_rgb24_upload,
};

static CoglGstRenderer rgb24_renderer =
{
  "RGB 24",
  COGL_GST_RGB24,
  0,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES (GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY,
                                                      "{ RGB, BGR }")),
  1, /* n_layers */
  cogl_gst_rgb24_setup_pipeline,
  cogl_gst_rgb24_upload,
  cogl_gst_dummy_shutdown,
};

static void
cogl_gst_rgb32_glsl_setup_pipeline (CoglGstVideoSink *sink,
                                    CoglPipeline *pipeline)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  static SnippetCache snippet_cache;
  SnippetCacheEntry *entry = get_layer_cache_entry (sink, &snippet_cache);

  if (entry == NULL)
    {
      char *source;

      source =
        g_strdup_printf ("vec4\n"
                         "cogl_gst_sample_video%i (vec2 UV)\n"
                         "{\n"
                         "  vec4 color = texture2D (cogl_sampler%i, UV);\n"
                         "  vec3 corrected = cogl_gst_get_corrected_color_from_rgb (color.rgb);\n"
                         /* Premultiply the color */
                         "  corrected.rgb *= color.a;\n"
                         "  return vec4(corrected.rgb, color.a);\n"
                         "}\n",
                         priv->custom_start,
                         priv->custom_start);

      entry = add_layer_cache_entry (sink, &snippet_cache, source);
      g_free (source);
    }

  setup_pipeline_from_cache_entry (sink, pipeline, entry, 1);
}

static void
cogl_gst_rgb32_setup_pipeline (CoglGstVideoSink *sink,
                               CoglPipeline *pipeline)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  char *layer_combine;

  setup_pipeline_from_cache_entry (sink, pipeline, NULL, 1);

  /* Premultiply the texture using the a special layer combine */
  layer_combine = g_strdup_printf ("RGB=MODULATE(PREVIOUS, TEXTURE_%i[A])\n"
                                   "A=REPLACE(PREVIOUS[A])",
                                   priv->custom_start);
  cogl_pipeline_set_layer_combine (pipeline,
                                   priv->custom_start + 1,
                                   layer_combine,
                                   NULL);
  g_free(layer_combine);
}

static CoglBool
cogl_gst_rgb32_upload (CoglGstVideoSink *sink,
                       GstBuffer *buffer)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  CoglPixelFormat format;
  GstVideoFrame frame;

  if (priv->bgr)
    format = COGL_PIXEL_FORMAT_BGRA_8888;
  else
    format = COGL_PIXEL_FORMAT_RGBA_8888;

  if (!gst_video_frame_map (&frame, &priv->info, buffer, GST_MAP_READ))
    goto map_fail;

  clear_frame_textures (sink);

  priv->frame[0] = video_texture_new_from_data (priv->ctx,
                                                GST_VIDEO_FRAME_COMP_WIDTH (&frame, 0),
                                                GST_VIDEO_FRAME_COMP_HEIGHT (&frame, 0),
                                                format,
                                                GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 0),
                                                GST_VIDEO_FRAME_PLANE_DATA (&frame, 0));

  gst_video_frame_unmap (&frame);

  return TRUE;

map_fail:
  {
    GST_ERROR_OBJECT (sink, "Could not map incoming video frame");
    return FALSE;
  }
}

static CoglGstRenderer rgb32_glsl_renderer =
{
  "RGB 32",
  COGL_GST_RGB32,
  COGL_GST_RENDERER_NEEDS_GLSL,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES(GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY,
                                                     "{ RGBA, BGRA }")),
  1, /* n_layers */
  cogl_gst_rgb32_glsl_setup_pipeline,
  cogl_gst_rgb32_upload,
};

static CoglGstRenderer rgb32_renderer =
{
  "RGB 32",
  COGL_GST_RGB32,
  0,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES(GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY,
                                                     "{ RGBA, BGRA }")),
  2, /* n_layers */
  cogl_gst_rgb32_setup_pipeline,
  cogl_gst_rgb32_upload,
  cogl_gst_dummy_shutdown,
};

static CoglBool
cogl_gst_yv12_upload (CoglGstVideoSink *sink,
                      GstBuffer *buffer)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  CoglPixelFormat format = COGL_PIXEL_FORMAT_A_8;
  GstVideoFrame frame;

  if (!gst_video_frame_map (&frame, &priv->info, buffer, GST_MAP_READ))
    goto map_fail;

  clear_frame_textures (sink);

  priv->frame[0] =
    video_texture_new_from_data (priv->ctx,
                                 GST_VIDEO_FRAME_COMP_WIDTH (&frame, 0),
                                 GST_VIDEO_FRAME_COMP_HEIGHT (&frame, 0),
                                 format,
                                 GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 0),
                                 GST_VIDEO_FRAME_PLANE_DATA (&frame, 0));

  priv->frame[2] =
    video_texture_new_from_data (priv->ctx,
                                 GST_VIDEO_FRAME_COMP_WIDTH (&frame, 1),
                                 GST_VIDEO_FRAME_COMP_HEIGHT (&frame, 1),
                                 format,
                                 GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 1),
                                 GST_VIDEO_FRAME_PLANE_DATA (&frame, 1));

  priv->frame[1] =
    video_texture_new_from_data (priv->ctx,
                                 GST_VIDEO_FRAME_COMP_WIDTH (&frame, 2),
                                 GST_VIDEO_FRAME_COMP_HEIGHT (&frame, 2),
                                 format,
                                 GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 2),
                                 GST_VIDEO_FRAME_PLANE_DATA (&frame, 2));

  gst_video_frame_unmap (&frame);

  return TRUE;

map_fail:
  {
    GST_ERROR_OBJECT (sink, "Could not map incoming video frame");
    return FALSE;
  }
}

static CoglBool
cogl_gst_i420_upload (CoglGstVideoSink *sink,
                      GstBuffer *buffer)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  CoglPixelFormat format = COGL_PIXEL_FORMAT_A_8;
  GstVideoFrame frame;

  if (!gst_video_frame_map (&frame, &priv->info, buffer, GST_MAP_READ))
    goto map_fail;

  clear_frame_textures (sink);

  priv->frame[0] =
    video_texture_new_from_data (priv->ctx,
                                 GST_VIDEO_FRAME_COMP_WIDTH (&frame, 0),
                                 GST_VIDEO_FRAME_COMP_HEIGHT (&frame, 0),
                                 format,
                                 GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 0),
                                 GST_VIDEO_FRAME_PLANE_DATA (&frame, 0));

  priv->frame[1] =
    video_texture_new_from_data (priv->ctx,
                                 GST_VIDEO_FRAME_COMP_WIDTH (&frame, 1),
                                 GST_VIDEO_FRAME_COMP_HEIGHT (&frame, 1),
                                 format,
                                 GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 1),
                                 GST_VIDEO_FRAME_PLANE_DATA (&frame, 1));

  priv->frame[2] =
    video_texture_new_from_data (priv->ctx,
                                 GST_VIDEO_FRAME_COMP_WIDTH (&frame, 2),
                                 GST_VIDEO_FRAME_COMP_HEIGHT (&frame, 2),
                                 format,
                                 GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 2),
                                 GST_VIDEO_FRAME_PLANE_DATA (&frame, 2));

  gst_video_frame_unmap (&frame);

  return TRUE;

map_fail:
  {
    GST_ERROR_OBJECT (sink, "Could not map incoming video frame");
    return FALSE;
  }
}

static void
cogl_gst_yv12_glsl_setup_pipeline (CoglGstVideoSink *sink,
                                   CoglPipeline *pipeline)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  static SnippetCache snippet_cache;
  SnippetCacheEntry *entry;

  entry = get_layer_cache_entry (sink, &snippet_cache);

  if (entry == NULL)
    {
      char *source;

      source =
        g_strdup_printf ("vec4\n"
                         "cogl_gst_sample_video%i (vec2 UV)\n"
                         "{\n"
                         "  float y = 1.1640625 * (texture2D (cogl_sampler%i, UV).a - 0.0625);\n"
                         "  float u = texture2D (cogl_sampler%i, UV).a - 0.5;\n"
                         "  float v = texture2D (cogl_sampler%i, UV).a - 0.5;\n"
                         "  vec3 corrected = cogl_gst_get_corrected_color_from_yuv (vec3 (y, u, v));\n"
                         "  vec4 color;\n"
                         "  color.rgb = cogl_gst_default_yuv_to_srgb (corrected);\n"
                         "  color.a = 1.0;\n"
                         "  return color;\n"
                         "}\n",
                         priv->video_start,
                         priv->video_start,
                         priv->video_start + 1,
                         priv->video_start + 2);

      entry = add_layer_cache_entry (sink, &snippet_cache, source);
      g_free (source);
    }

  setup_pipeline_from_cache_entry (sink, pipeline, entry, 3);
}

static CoglGstRenderer yv12_glsl_renderer =
{
  "YV12 glsl",
  COGL_GST_YV12,
  COGL_GST_RENDERER_NEEDS_GLSL,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES(GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY,
                                                     "YV12")),
  3, /* n_layers */
  cogl_gst_yv12_glsl_setup_pipeline,
  cogl_gst_yv12_upload,
  cogl_gst_dummy_shutdown,
};

static CoglGstRenderer i420_glsl_renderer =
{
  "I420 glsl",
  COGL_GST_I420,
  COGL_GST_RENDERER_NEEDS_GLSL,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES(GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY,
                                                     "I420")),
  3, /* n_layers */
  cogl_gst_yv12_glsl_setup_pipeline,
  cogl_gst_i420_upload,
  cogl_gst_dummy_shutdown,
};

static void
cogl_gst_ayuv_glsl_setup_pipeline (CoglGstVideoSink *sink,
                                   CoglPipeline *pipeline)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  static SnippetCache snippet_cache;
  SnippetCacheEntry *entry;

  entry = get_layer_cache_entry (sink, &snippet_cache);

  if (entry == NULL)
    {
      char *source;

      source
        = g_strdup_printf ("vec4\n"
                           "cogl_gst_sample_video%i (vec2 UV)\n"
                           "{\n"
                           "  vec4 color = texture2D (cogl_sampler%i, UV);\n"
                           "  float y = 1.1640625 * (color.g - 0.0625);\n"
                           "  float u = color.b - 0.5;\n"
                           "  float v = color.a - 0.5;\n"
                           "  vec3 corrected = cogl_gst_get_corrected_color_from_yuv (vec3 (y, u, v));\n"
                           "  color.a = color.r;\n"
                           "  color.rgb = cogl_gst_default_yuv_to_srgb (corrected);\n"
                           /* Premultiply the color */
                           "  color.rgb *= color.a;\n"
                           "  return color;\n"
                           "}\n",
                           priv->video_start,
                           priv->video_start);

      entry = add_layer_cache_entry (sink, &snippet_cache, source);
      g_free (source);
    }

  setup_pipeline_from_cache_entry (sink, pipeline, entry, 1);
}

static CoglBool
cogl_gst_ayuv_upload (CoglGstVideoSink *sink,
                      GstBuffer *buffer)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  CoglPixelFormat format = COGL_PIXEL_FORMAT_RGBA_8888;
  GstVideoFrame frame;

  if (!gst_video_frame_map (&frame, &priv->info, buffer, GST_MAP_READ))
    goto map_fail;

  clear_frame_textures (sink);

  priv->frame[0] = video_texture_new_from_data (priv->ctx,
                                                GST_VIDEO_FRAME_COMP_WIDTH (&frame, 0),
                                                GST_VIDEO_FRAME_COMP_HEIGHT (&frame, 0),
                                                format,
                                                GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 0),
                                                GST_VIDEO_FRAME_PLANE_DATA (&frame, 0));

  gst_video_frame_unmap (&frame);

  return TRUE;

map_fail:
  {
    GST_ERROR_OBJECT (sink, "Could not map incoming video frame");
    return FALSE;
  }
}

static CoglGstRenderer ayuv_glsl_renderer =
{
  "AYUV glsl",
  COGL_GST_AYUV,
  COGL_GST_RENDERER_NEEDS_GLSL,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES(GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY,
                                                     "AYUV")),
  1, /* n_layers */
  cogl_gst_ayuv_glsl_setup_pipeline,
  cogl_gst_ayuv_upload,
  cogl_gst_dummy_shutdown,
};

static void
cogl_gst_nv12_glsl_setup_pipeline (CoglGstVideoSink *sink,
                                   CoglPipeline *pipeline)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  static SnippetCache snippet_cache;
  SnippetCacheEntry *entry;

  entry = get_layer_cache_entry (sink, &snippet_cache);

  if (entry == NULL)
    {
      char *source;

      source =
        g_strdup_printf ("vec4\n"
                         "cogl_gst_sample_video%i (vec2 UV)\n"
                         "{\n"
                         "  vec4 color;\n"
                         "  float y = 1.1640625 *\n"
                         "            (texture2D (cogl_sampler%i, UV).a -\n"
                         "             0.0625);\n"
                         "  vec2 uv = texture2D (cogl_sampler%i, UV).rg;\n"
                         "  uv -= 0.5;\n"
                         "  float u = uv.x;\n"
                         "  float v = uv.y;\n"
                         "  vec3 corrected = cogl_gst_get_corrected_color_from_yuv (vec3 (y, u, v));\n"
                         "  color.rgb = cogl_gst_default_yuv_to_srgb (corrected);\n"
                         "  color.a = 1.0;\n"
                         "  return color;\n"
                         "}\n",
                         priv->custom_start,
                         priv->custom_start,
                         priv->custom_start + 1);

      entry = add_layer_cache_entry (sink, &snippet_cache, source);
      g_free (source);
    }

  setup_pipeline_from_cache_entry (sink, pipeline, entry, 2);
}

static CoglBool
cogl_gst_nv12_upload (CoglGstVideoSink *sink,
                      GstBuffer *buffer)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  GstVideoFrame frame;

  if (!gst_video_frame_map (&frame, &priv->info, buffer, GST_MAP_READ))
    goto map_fail;

  clear_frame_textures (sink);

  priv->frame[0] =
    video_texture_new_from_data (priv->ctx,
                                 GST_VIDEO_INFO_COMP_WIDTH (&priv->info, 0),
                                 GST_VIDEO_INFO_COMP_HEIGHT (&priv->info, 0),
                                 COGL_PIXEL_FORMAT_A_8,
                                 priv->info.stride[0],
                                 frame.data[0]);

  priv->frame[1] =
    video_texture_new_from_data (priv->ctx,
                                 GST_VIDEO_INFO_COMP_WIDTH (&priv->info, 1),
                                 GST_VIDEO_INFO_COMP_HEIGHT (&priv->info, 1),
                                 COGL_PIXEL_FORMAT_RG_88,
                                 priv->info.stride[1],
                                 frame.data[1]);

  gst_video_frame_unmap (&frame);

  return TRUE;

 map_fail:
  {
    GST_ERROR_OBJECT (sink, "Could not map incoming video frame");
    return FALSE;
  }
}

static CoglGstRenderer nv12_glsl_renderer =
{
  "NV12 glsl",
  COGL_GST_NV12,
  COGL_GST_RENDERER_NEEDS_GLSL | COGL_GST_RENDERER_NEEDS_TEXTURE_RG,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES(GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY,
                                                     "NV12")),
  2, /* n_layers */
  cogl_gst_nv12_glsl_setup_pipeline,
  cogl_gst_nv12_upload,
  cogl_gst_dummy_shutdown,
};

static GSList*
cogl_gst_build_renderers_list (CoglContext *ctx)
{
  GSList *list = NULL;
  CoglGstRendererFlag flags = 0;
  int i;
  static CoglGstRenderer *const renderers[] =
  {
    /* These are in increasing order of priority so that the
     * priv->renderers will be in decreasing order. That way the GLSL
     * renderers will be preferred if they are available */
    &rgb24_renderer,
    &rgb32_renderer,
    &rgb24_glsl_renderer,
    &rgb32_glsl_renderer,
    &yv12_glsl_renderer,
    &i420_glsl_renderer,
    &ayuv_glsl_renderer,
    &nv12_glsl_renderer,
    NULL
  };

  if (cogl_has_feature (ctx, COGL_FEATURE_ID_GLSL))
    flags |= COGL_GST_RENDERER_NEEDS_GLSL;

  if (cogl_has_feature (ctx, COGL_FEATURE_ID_TEXTURE_RG))
    flags |= COGL_GST_RENDERER_NEEDS_TEXTURE_RG;

  for (i = 0; renderers[i]; i++)
    if ((renderers[i]->flags & flags) == renderers[i]->flags)
      list = g_slist_prepend (list, renderers[i]);

  return list;
}

static void
append_cap (gpointer data,
            gpointer user_data)
{
  CoglGstRenderer *renderer = (CoglGstRenderer *) data;
  GstCaps *caps = (GstCaps *) user_data;
  GstCaps *writable_caps;
  writable_caps =
    gst_caps_make_writable (gst_static_caps_get (&renderer->caps));
  gst_caps_append (caps, writable_caps);
}

static GstCaps *
cogl_gst_build_caps (GSList *renderers)
{
  GstCaps *caps;

  caps = gst_caps_new_empty ();

  g_slist_foreach (renderers, append_cap, caps);

  return caps;
}

void
cogl_gst_video_sink_set_context (CoglGstVideoSink *vt,
                                 CoglContext *ctx)
{
  CoglGstVideoSinkPrivate *priv = vt->priv;

  if (ctx)
    ctx = cogl_object_ref (ctx);

  if (priv->ctx)
    {
      cogl_object_unref (priv->ctx);
      g_slist_free (priv->renderers);
      priv->renderers = NULL;
      if (priv->caps)
        {
          gst_caps_unref (priv->caps);
          priv->caps = NULL;
        }
    }

  if (ctx)
    {
      priv->ctx = ctx;
      priv->renderers = cogl_gst_build_renderers_list (priv->ctx);
      priv->caps = cogl_gst_build_caps (priv->renderers);
    }
}

static CoglGstRenderer *
cogl_gst_find_renderer_by_format (CoglGstVideoSink *sink,
                                  CoglGstVideoFormat format)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  CoglGstRenderer *renderer = NULL;
  GSList *element;

  /* The renderers list is in decreasing order of priority so we'll
   * pick the first one that matches */
  for (element = priv->renderers; element; element = g_slist_next (element))
    {
      CoglGstRenderer *candidate = (CoglGstRenderer *) element->data;
      if (candidate->format == format)
        {
          renderer = candidate;
          break;
        }
    }

  return renderer;
}

static GstCaps *
cogl_gst_video_sink_get_caps (GstBaseSink *bsink,
                              GstCaps *filter)
{
  CoglGstVideoSink *sink;
  sink = COGL_GST_VIDEO_SINK (bsink);

  if (sink->priv->caps == NULL)
    return NULL;
  else
    return gst_caps_ref (sink->priv->caps);
}

static CoglBool
cogl_gst_video_sink_parse_caps (GstCaps *caps,
                                CoglGstVideoSink *sink,
                                CoglBool save)
{
  CoglGstVideoSinkPrivate *priv = sink->priv;
  GstCaps *intersection;
  GstVideoInfo vinfo;
  CoglGstVideoFormat format;
  CoglBool bgr = FALSE;
  CoglGstRenderer *renderer;

  intersection = gst_caps_intersect (priv->caps, caps);
  if (gst_caps_is_empty (intersection))
    goto no_intersection;

  gst_caps_unref (intersection);

  if (!gst_video_info_from_caps (&vinfo, caps))
    goto unknown_format;

  switch (vinfo.finfo->format)
    {
    case GST_VIDEO_FORMAT_YV12:
      format = COGL_GST_YV12;
      break;
    case GST_VIDEO_FORMAT_I420:
      format = COGL_GST_I420;
      break;
    case GST_VIDEO_FORMAT_AYUV:
      format = COGL_GST_AYUV;
      bgr = FALSE;
      break;
    case GST_VIDEO_FORMAT_NV12:
      format = COGL_GST_NV12;
      break;
    case GST_VIDEO_FORMAT_RGB:
      format = COGL_GST_RGB24;
      bgr = FALSE;
      break;
    case GST_VIDEO_FORMAT_BGR:
      format = COGL_GST_RGB24;
      bgr = TRUE;
      break;
    case GST_VIDEO_FORMAT_RGBA:
      format = COGL_GST_RGB32;
      bgr = FALSE;
      break;
    case GST_VIDEO_FORMAT_BGRA:
      format = COGL_GST_RGB32;
      bgr = TRUE;
      break;
    default:
      goto unhandled_format;
    }

  renderer = cogl_gst_find_renderer_by_format (sink, format);

  if (G_UNLIKELY (renderer == NULL))
    goto no_suitable_renderer;

  GST_INFO_OBJECT (sink, "found the %s renderer", renderer->name);

  if (save)
    {
      priv->info = vinfo;

      priv->format = format;
      priv->bgr = bgr;

      priv->renderer = renderer;
    }

  return TRUE;


no_intersection:
  {
    GST_WARNING_OBJECT (sink,
        "Incompatible caps, don't intersect with %" GST_PTR_FORMAT, priv->caps);
    return FALSE;
  }

unknown_format:
  {
    GST_WARNING_OBJECT (sink, "Could not figure format of input caps");
    return FALSE;
  }

unhandled_format:
  {
    GST_ERROR_OBJECT (sink, "Provided caps aren't supported by clutter-gst");
    return FALSE;
  }

no_suitable_renderer:
  {
    GST_ERROR_OBJECT (sink, "could not find a suitable renderer");
    return FALSE;
  }
}

static CoglBool
cogl_gst_video_sink_set_caps (GstBaseSink *bsink,
                              GstCaps *caps)
{
  CoglGstVideoSink *sink;
  CoglGstVideoSinkPrivate *priv;

  sink = COGL_GST_VIDEO_SINK (bsink);
  priv = sink->priv;

  if (!cogl_gst_video_sink_parse_caps (caps, sink, FALSE))
    return FALSE;

  g_mutex_lock (&priv->source->buffer_lock);
  priv->source->has_new_caps = TRUE;
  g_mutex_unlock (&priv->source->buffer_lock);

  return TRUE;
}

static CoglBool
cogl_gst_source_dispatch (GSource *source,
                          GSourceFunc callback,
                          void *user_data)
{
  CoglGstSource *gst_source= (CoglGstSource*) source;
  CoglGstVideoSinkPrivate *priv = gst_source->sink->priv;
  GstBuffer *buffer;
  gboolean pipeline_ready = FALSE;

  g_mutex_lock (&gst_source->buffer_lock);

  if (G_UNLIKELY (gst_source->has_new_caps))
    {
      GstCaps *caps =
        gst_pad_get_current_caps (GST_BASE_SINK_PAD ((GST_BASE_SINK
                (gst_source->sink))));

      if (!cogl_gst_video_sink_parse_caps (caps, gst_source->sink, TRUE))
        goto negotiation_fail;

      gst_source->has_new_caps = FALSE;
      priv->free_layer = _cogl_gst_video_sink_get_free_layer (gst_source->sink);

      dirty_default_pipeline (gst_source->sink);

      /* We are now in a state where we could generate the pipeline if
       * the application requests it so we can emit the signal.
       * However we'll actually generate the pipeline lazily only if
       * the application actually asks for it. */
      pipeline_ready = TRUE;
    }

  buffer = gst_source->buffer;
  gst_source->buffer = NULL;

  g_mutex_unlock (&gst_source->buffer_lock);

  if (buffer)
    {
      if (!priv->renderer->upload (gst_source->sink, buffer))
        goto fail_upload;

      priv->had_upload_once = TRUE;

      gst_buffer_unref (buffer);
    }
  else
    GST_WARNING_OBJECT (gst_source->sink, "No buffers available for display");

  if (G_UNLIKELY (pipeline_ready))
    g_signal_emit (gst_source->sink,
                   video_sink_signals[PIPELINE_READY_SIGNAL],
                   0 /* detail */);
  if (priv->had_upload_once)
    g_signal_emit (gst_source->sink,
                   video_sink_signals[NEW_FRAME_SIGNAL], 0,
                   NULL);

  return TRUE;


negotiation_fail:
  {
    GST_WARNING_OBJECT (gst_source->sink,
        "Failed to handle caps. Stopping GSource");
    priv->flow_return = GST_FLOW_NOT_NEGOTIATED;
    g_mutex_unlock (&gst_source->buffer_lock);

    return FALSE;
  }

fail_upload:
  {
    GST_WARNING_OBJECT (gst_source->sink, "Failed to upload buffer");
    priv->flow_return = GST_FLOW_ERROR;
    gst_buffer_unref (buffer);
    return FALSE;
  }
}

static GSourceFuncs gst_source_funcs =
{
  cogl_gst_source_prepare,
  cogl_gst_source_check,
  cogl_gst_source_dispatch,
  cogl_gst_source_finalize
};

static CoglGstSource *
cogl_gst_source_new (CoglGstVideoSink *sink)
{
  GSource *source;
  CoglGstSource *gst_source;

  source = g_source_new (&gst_source_funcs, sizeof (CoglGstSource));
  gst_source = (CoglGstSource *) source;

  g_source_set_can_recurse (source, TRUE);
  g_source_set_priority (source, COGL_GST_DEFAULT_PRIORITY);

  gst_source->sink = sink;
  g_mutex_init (&gst_source->buffer_lock);
  gst_source->buffer = NULL;

  return gst_source;
}

static void
cogl_gst_video_sink_init (CoglGstVideoSink *sink)
{
  CoglGstVideoSinkPrivate *priv;

  sink->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE (sink,
                                                   COGL_GST_TYPE_VIDEO_SINK,
                                                   CoglGstVideoSinkPrivate);
  priv->custom_start = 0;
  priv->default_sample = TRUE;

  priv->brightness = DEFAULT_BRIGHTNESS;
  priv->contrast = DEFAULT_CONTRAST;
  priv->hue = DEFAULT_HUE;
  priv->saturation = DEFAULT_SATURATION;

  priv->tabley = g_new0 (guint8, 256);
  priv->tableu = g_new0 (guint8, 256 * 256);
  priv->tablev = g_new0 (guint8, 256 * 256);
}

static GstFlowReturn
_cogl_gst_video_sink_render (GstBaseSink *bsink,
                             GstBuffer *buffer)
{
  CoglGstVideoSink *sink = COGL_GST_VIDEO_SINK (bsink);
  CoglGstVideoSinkPrivate *priv = sink->priv;
  CoglGstSource *gst_source = priv->source;

  g_mutex_lock (&gst_source->buffer_lock);

  if (G_UNLIKELY (priv->flow_return != GST_FLOW_OK))
    goto dispatch_flow_ret;

  if (gst_source->buffer)
    gst_buffer_unref (gst_source->buffer);

  gst_source->buffer = gst_buffer_ref (buffer);
  g_mutex_unlock (&gst_source->buffer_lock);

  g_main_context_wakeup (NULL);

  return GST_FLOW_OK;

  dispatch_flow_ret:
  {
    g_mutex_unlock (&gst_source->buffer_lock);
    return priv->flow_return;
  }
}

static void
cogl_gst_video_sink_dispose (GObject *object)
{
  CoglGstVideoSink *self;
  CoglGstVideoSinkPrivate *priv;

  self = COGL_GST_VIDEO_SINK (object);
  priv = self->priv;

  clear_frame_textures (self);

  if (priv->renderer) {
    priv->renderer->shutdown (self);
    priv->renderer = NULL;
  }

  if (priv->pipeline)
    {
      cogl_object_unref (priv->pipeline);
      priv->pipeline = NULL;
    }

  if (priv->caps)
    {
      gst_caps_unref (priv->caps);
      priv->caps = NULL;
    }

  if (priv->tabley)
    {
      g_free (priv->tabley);
      priv->tabley = NULL;
    }

  if (priv->tableu)
    {
      g_free (priv->tableu);
      priv->tableu = NULL;
    }

  if (priv->tablev)
    {
      g_free (priv->tablev);
      priv->tablev = NULL;
    }

  G_OBJECT_CLASS (cogl_gst_video_sink_parent_class)->dispose (object);
}

static void
cogl_gst_video_sink_finalize (GObject *object)
{
  CoglGstVideoSink *self = COGL_GST_VIDEO_SINK (object);

  cogl_gst_video_sink_set_context (self, NULL);

  G_OBJECT_CLASS (cogl_gst_video_sink_parent_class)->finalize (object);
}

static CoglBool
cogl_gst_video_sink_start (GstBaseSink *base_sink)
{
  CoglGstVideoSink *sink = COGL_GST_VIDEO_SINK (base_sink);
  CoglGstVideoSinkPrivate *priv = sink->priv;

  priv->source = cogl_gst_source_new (sink);
  g_source_attach ((GSource *) priv->source, NULL);
  priv->flow_return = GST_FLOW_OK;
  return TRUE;
}

static void
cogl_gst_video_sink_set_property (GObject *object,
                                  unsigned int prop_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
  CoglGstVideoSink *sink = COGL_GST_VIDEO_SINK (object);

  switch (prop_id)
    {
    case PROP_UPDATE_PRIORITY:
      cogl_gst_video_sink_set_priority (sink, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
cogl_gst_video_sink_get_property (GObject *object,
                                  unsigned int prop_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
  CoglGstVideoSink *sink = COGL_GST_VIDEO_SINK (object);
  CoglGstVideoSinkPrivate *priv = sink->priv;

  switch (prop_id)
    {
    case PROP_UPDATE_PRIORITY:
      g_value_set_int (value, g_source_get_priority ((GSource *) priv->source));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static CoglBool
cogl_gst_video_sink_stop (GstBaseSink *base_sink)
{
  CoglGstVideoSink *sink = COGL_GST_VIDEO_SINK (base_sink);
  CoglGstVideoSinkPrivate *priv = sink->priv;

  if (priv->source)
    {
      GSource *source = (GSource *) priv->source;
      g_source_destroy (source);
      g_source_unref (source);
      priv->source = NULL;
    }

  return TRUE;
}

static gboolean
cogl_gst_video_sink_propose_allocation (GstBaseSink *base_sink, GstQuery *query)
{
  gboolean need_pool = FALSE;
  GstCaps *caps = NULL;

  gst_query_parse_allocation (query, &caps, &need_pool);

  gst_query_add_allocation_meta (query,
                                 GST_VIDEO_META_API_TYPE, NULL);

  return TRUE;
}

static void
cogl_gst_video_sink_class_init (CoglGstVideoSinkClass *klass)
{
  GObjectClass *go_class = G_OBJECT_CLASS (klass);
  GstBaseSinkClass *gb_class = GST_BASE_SINK_CLASS (klass);
  GstElementClass *ge_class = GST_ELEMENT_CLASS (klass);
  GstPadTemplate *pad_template;
  GParamSpec *pspec;

  g_type_class_add_private (klass, sizeof (CoglGstVideoSinkPrivate));
  go_class->set_property = cogl_gst_video_sink_set_property;
  go_class->get_property = cogl_gst_video_sink_get_property;
  go_class->dispose = cogl_gst_video_sink_dispose;
  go_class->finalize = cogl_gst_video_sink_finalize;

  pad_template = gst_static_pad_template_get (&sinktemplate_all);
  gst_element_class_add_pad_template (ge_class, pad_template);

  gst_element_class_set_metadata (ge_class,
                                  "Cogl video sink", "Sink/Video",
                                  "Sends video data from GStreamer to a "
                                  "Cogl pipeline",
                                  "Jonathan Matthew <jonathan@kaolin.wh9.net>, "
                                  "Matthew Allum <mallum@o-hand.com, "
                                  "Chris Lord <chris@o-hand.com>, "
                                  "Plamena Manolova "
                                  "<plamena.n.manolova@intel.com>");

  gb_class->render = _cogl_gst_video_sink_render;
  gb_class->preroll = _cogl_gst_video_sink_render;
  gb_class->start = cogl_gst_video_sink_start;
  gb_class->stop = cogl_gst_video_sink_stop;
  gb_class->set_caps = cogl_gst_video_sink_set_caps;
  gb_class->get_caps = cogl_gst_video_sink_get_caps;
  gb_class->propose_allocation = cogl_gst_video_sink_propose_allocation;

  pspec = g_param_spec_int ("update-priority",
                            "Update Priority",
                            "Priority of video updates in the thread",
                            -G_MAXINT, G_MAXINT,
                            COGL_GST_DEFAULT_PRIORITY,
                            COGL_GST_PARAM_READWRITE);

  g_object_class_install_property (go_class, PROP_UPDATE_PRIORITY, pspec);

  video_sink_signals[PIPELINE_READY_SIGNAL] =
    g_signal_new ("pipeline-ready",
                  COGL_GST_TYPE_VIDEO_SINK,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (CoglGstVideoSinkClass, pipeline_ready),
                  NULL, /* accumulator */
                  NULL, /* accu_data */
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0 /* n_params */);

  video_sink_signals[NEW_FRAME_SIGNAL] =
    g_signal_new ("new-frame",
                  COGL_GST_TYPE_VIDEO_SINK,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (CoglGstVideoSinkClass, new_frame),
                  NULL, /* accumulator */
                  NULL, /* accu_data */
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0 /* n_params */);
}

CoglGstVideoSink *
cogl_gst_video_sink_new (CoglContext *ctx)
{
  CoglGstVideoSink *sink = g_object_new (COGL_GST_TYPE_VIDEO_SINK, NULL);
  cogl_gst_video_sink_set_context (sink, ctx);

  return sink;
}

float
cogl_gst_video_sink_get_aspect (CoglGstVideoSink *vt)
{
  GstVideoInfo *info;

  g_return_val_if_fail (COGL_GST_IS_VIDEO_SINK (vt), 0.);

  info = &vt->priv->info;
  return ((float)info->width * (float)info->par_n) /
    ((float)info->height * (float)info->par_d);
}

float
cogl_gst_video_sink_get_width_for_height (CoglGstVideoSink *vt,
                                          float height)
{
  float aspect;

  g_return_val_if_fail (COGL_GST_IS_VIDEO_SINK (vt), 0.);

  aspect = cogl_gst_video_sink_get_aspect (vt);
  return height * aspect;
}

float
cogl_gst_video_sink_get_height_for_width (CoglGstVideoSink *vt,
                                          float width)
{
  float aspect;

  g_return_val_if_fail (COGL_GST_IS_VIDEO_SINK (vt), 0.);

  aspect = cogl_gst_video_sink_get_aspect (vt);
  return width / aspect;
}

void
cogl_gst_video_sink_fit_size (CoglGstVideoSink *vt,
                              const CoglGstRectangle *available,
                              CoglGstRectangle *output)
{
  g_return_if_fail (COGL_GST_IS_VIDEO_SINK (vt));
  g_return_if_fail (available != NULL);
  g_return_if_fail (output != NULL);

  if (available->height == 0.0f)
    {
      output->x = available->x;
      output->y = available->y;
      output->width = output->height = 0;
    }
  else
    {
      float available_aspect = available->width / available->height;
      float video_aspect = cogl_gst_video_sink_get_aspect (vt);

      if (video_aspect > available_aspect)
        {
          output->width = available->width;
          output->height = available->width / video_aspect;
          output->x = available->x;
          output->y = available->y + (available->height - output->height) / 2;
        }
      else
        {
          output->width = available->height * video_aspect;
          output->height = available->height;
          output->x = available->x + (available->width - output->width) / 2;
          output->y = available->y;
        }
    }
}

void
cogl_gst_video_sink_get_natural_size (CoglGstVideoSink *vt,
                                      float *width,
                                      float *height)
{
  GstVideoInfo *info;

  g_return_if_fail (COGL_GST_IS_VIDEO_SINK (vt));

  info = &vt->priv->info;

  if (info->par_n > info->par_d)
    {
      /* To display the video at the right aspect ratio then in this
       * case the pixels need to be stretched horizontally and so we
       * use the unscaled height as our reference.
       */

      if (height)
        *height = info->height;
      if (width)
        *width = cogl_gst_video_sink_get_width_for_height (vt, info->height);
    }
  else
    {
      if (width)
        *width = info->width;
      if (height)
        *height = cogl_gst_video_sink_get_height_for_width (vt, info->width);
    }
}

float
cogl_gst_video_sink_get_natural_width (CoglGstVideoSink *vt)
{
  float width;
  cogl_gst_video_sink_get_natural_size (vt, &width, NULL);
  return width;
}

float
cogl_gst_video_sink_get_natural_height (CoglGstVideoSink *vt)
{
  float height;
  cogl_gst_video_sink_get_natural_size (vt, NULL, &height);
  return height;
}

CoglBool
cogl_gst_video_sink_is_ready (CoglGstVideoSink *sink)
{
  return !!sink->priv->renderer;
}
