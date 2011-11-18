/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2011 Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *
 *
 * Authors:
 *   Robert Bragg <robert@linux.intel.com>
 */

#ifndef __COGL_FRAMEBUFFER_H
#define __COGL_FRAMEBUFFER_H

#include <cogl/cogl.h>

#include <glib.h>

#ifdef COGL_HAS_WIN32_SUPPORT
#include <windows.h>
#endif /* COGL_HAS_WIN32_SUPPORT */

#if defined (COGL_HAS_EGL_PLATFORM_WAYLAND_SUPPORT)
#include <wayland-client.h>
#endif /* COGL_HAS_EGL_PLATFORM_WAYLAND_SUPPORT */

G_BEGIN_DECLS

/**
 * SECTION:cogl-framebuffer
 * @short_description: A common interface for manipulating framebuffers
 *
 * Framebuffers are a collection of buffers that can be rendered too.
 * A framebuffer may be comprised of one or more color buffers, an
 * optional depth buffer and an optional stencil buffer. Other
 * configuration parameters are associated with framebuffers too such
 * as whether the framebuffer supports multi-sampling (an anti-aliasing
 * technique) or dithering.
 *
 * There are two kinds of framebuffer in Cogl, #CoglOnscreen
 * framebuffers and #CoglOffscreen framebuffers. As the names imply
 * offscreen framebuffers are for rendering something offscreen
 * (perhaps to a texture which is bound as one of the color buffers).
 * The exact semantics of onscreen framebuffers depends on the window
 * system backend that you are using, but typically you can expect
 * rendering to a #CoglOnscreen framebuffer will be immediately
 * visible to the user.
 *
 * If you want to create a new framebuffer then you should start by
 * looking at the #CoglOnscreen and #CoglOffscreen constructor
 * functions, such as cogl_offscreen_new_to_texture() or
 * cogl_onscreen_new(). The #CoglFramebuffer interface deals with
 * all aspects that are common between those two types of framebuffer.
 *
 * Setup of a new CoglFramebuffer happens in two stages. There is a
 * configuration stage where you specify all the options and ancillary
 * buffers you want associated with your framebuffer and then when you
 * are happy with the configuration you can "allocate" the framebuffer
 * using cogl_framebuffer_allocate (). Technically explicitly calling
 * cogl_framebuffer_allocate () is optional for convenience and the
 * framebuffer will automatically be allocated when you first try to
 * draw to it, but if you do the allocation manually then you can
 * also catch any possible errors that may arise from your
 * configuration.
 */

#ifdef COGL_ENABLE_EXPERIMENTAL_API
#define cogl_onscreen_new cogl_onscreen_new_EXP

#define COGL_FRAMEBUFFER(X) ((CoglFramebuffer *)(X))

#define cogl_framebuffer_allocate cogl_framebuffer_allocate_EXP
gboolean
cogl_framebuffer_allocate (CoglFramebuffer *framebuffer,
                           GError **error);

#define cogl_framebuffer_get_width cogl_framebuffer_get_width_EXP
int
cogl_framebuffer_get_width (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_get_height cogl_framebuffer_get_height_EXP
int
cogl_framebuffer_get_height (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_set_viewport cogl_framebuffer_set_viewport_EXP
void
cogl_framebuffer_set_viewport (CoglFramebuffer *framebuffer,
                               float x,
                               float y,
                               float width,
                               float height);

#define cogl_framebuffer_get_viewport_x cogl_framebuffer_get_viewport_x_EXP
float
cogl_framebuffer_get_viewport_x (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_get_viewport_y cogl_framebuffer_get_viewport_y_EXP
float
cogl_framebuffer_get_viewport_y (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_get_viewport_width cogl_framebuffer_get_viewport_width_EXP
float
cogl_framebuffer_get_viewport_width (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_get_viewport_height cogl_framebuffer_get_viewport_height_EXP
float
cogl_framebuffer_get_viewport_height (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_get_viewport4fv cogl_framebuffer_get_viewport4fv_EXP
void
cogl_framebuffer_get_viewport4fv (CoglFramebuffer *framebuffer,
                                  float *viewport);

#define cogl_framebuffer_push_matrix cogl_framebuffer_push_matrix_EXP
/**
 * cogl_framebuffer_push_matrix:
 * @framebuffer: A #CoglFramebuffer pointer
 *
 * Copies the current model-view matrix onto the matrix stack. The matrix
 * can later be restored with cogl_framebuffer_pop_matrix().
 *
 * Since: 1.10
 */
void
cogl_framebuffer_push_matrix (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_pop_matrix cogl_framebuffer_pop_matrix_EXP
/**
 * cogl_framebuffer_pop_matrix:
 * @framebuffer: A #CoglFramebuffer pointer
 *
 * Restores the model-view matrix on the top of the matrix stack.
 *
 * Since: 1.10
 */
void
cogl_framebuffer_pop_matrix (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_identity_matrix \
  cogl_framebuffer_identity_matrix_EXP
/**
 * cogl_framebuffer_identity_matrix:
 * @framebuffer: A #CoglFramebuffer pointer
 *
 * Resets the current model-view matrix to the identity matrix.
 *
 * Since: 1.10
 * Stability: unstable
 */
void
cogl_framebuffer_identity_matrix (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_scale cogl_framebuffer_scale_EXP
/**
 * cogl_framebuffer_scale:
 * @framebuffer: A #CoglFramebuffer pointer
 * @x: Amount to scale along the x-axis
 * @y: Amount to scale along the y-axis
 * @z: Amount to scale along the z-axis
 *
 * Multiplies the current model-view matrix by one that scales the x,
 * y and z axes by the given values.
 *
 * Since: 1.10
 */
void
cogl_framebuffer_scale (CoglFramebuffer *framebuffer,
                        float x,
                        float y,
                        float z);

#define cogl_framebuffer_translate cogl_framebuffer_translate_EXP
/**
 * cogl_framebuffer_translate:
 * @framebuffer: A #CoglFramebuffer pointer
 * @x: Distance to translate along the x-axis
 * @y: Distance to translate along the y-axis
 * @z: Distance to translate along the z-axis
 *
 * Multiplies the current model-view matrix by one that translates the
 * model along all three axes according to the given values.
 *
 * Since: 1.10
 */
void
cogl_framebuffer_translate (CoglFramebuffer *framebuffer,
                            float x,
                            float y,
                            float z);

#define cogl_framebuffer_rotate cogl_framebuffer_rotate_EXP
/**
 * cogl_framebuffer_rotate:
 * @framebuffer: A #CoglFramebuffer pointer
 * @angle: Angle in degrees to rotate.
 * @x: X-component of vertex to rotate around.
 * @y: Y-component of vertex to rotate around.
 * @z: Z-component of vertex to rotate around.
 *
 * Multiplies the current model-view matrix by one that rotates the
 * model around the vertex specified by @x, @y and @z. The rotation
 * follows the right-hand thumb rule so for example rotating by 10
 * degrees about the vertex (0, 0, 1) causes a small counter-clockwise
 * rotation.
 *
 * Since: 1.10
 */
void
cogl_framebuffer_rotate (CoglFramebuffer *framebuffer,
                         float angle,
                         float x,
                         float y,
                         float z);

#define cogl_framebuffer_transform cogl_framebuffer_transform_EXP
/**
 * cogl_framebuffer_transform:
 * @framebuffer: A #CoglFramebuffer pointer
 * @matrix: the matrix to multiply with the current model-view
 *
 * Multiplies the current model-view matrix by the given matrix.
 *
 * Since: 1.10
 */
void
cogl_framebuffer_transform (CoglFramebuffer *framebuffer,
                            const CoglMatrix *matrix);

#define cogl_framebuffer_get_modelview_matrix \
  cogl_framebuffer_get_modelview_matrix_EXP
/**
 * cogl_framebuffer_get_modelview_matrix:
 * @framebuffer: A #CoglFramebuffer pointer
 * @matrix: (out): return location for the model-view matrix
 *
 * Stores the current model-view matrix in @matrix.
 *
 * Since: 1.10
 */
void
cogl_framebuffer_get_modelview_matrix (CoglFramebuffer *framebuffer,
                                       CoglMatrix *matrix);

#define cogl_framebuffer_set_modelview_matrix \
  cogl_framebuffer_set_modelview_matrix_EXP
/**
 * cogl_framebuffer_set_modelview_matrix:
 * @framebuffer: A #CoglFramebuffer pointer
 * @matrix: the new model-view matrix
 *
 * Sets @matrix as the new model-view matrix.
 *
 * Since: 1.10
 */
void
cogl_framebuffer_set_modelview_matrix (CoglFramebuffer *framebuffer,
                                       CoglMatrix *matrix);

#define cogl_framebuffer_perspective cogl_framebuffer_perspective_EXP
/**
 * cogl_framebuffer_perspective:
 * @framebuffer: A #CoglFramebuffer pointer
 * @fovy: Vertical field of view angle in degrees.
 * @aspect: The (width over height) aspect ratio for display
 * @z_near: The distance to the near clipping plane (Must be positive,
 *   and must not be 0)
 * @z_far: The distance to the far clipping plane (Must be positive)
 *
 * Replaces the current projection matrix with a perspective matrix
 * based on the provided values.
 *
 * <note>You should be careful not to have to great a @z_far / @z_near
 * ratio since that will reduce the effectiveness of depth testing
 * since there wont be enough precision to identify the depth of
 * objects near to each other.</note>
 *
 * Since: 1.10
 */
void
cogl_framebuffer_perspective (CoglFramebuffer *framebuffer,
                              float fov_y,
                              float aspect,
                              float z_near,
                              float z_far);

#define cogl_framebuffer_frustum cogl_framebuffer_frustum_EXP
/**
 * cogl_framebuffer_frustum:
 * @framebuffer: A #CoglFramebuffer pointer
 * @left: X position of the left clipping plane where it
 *   intersects the near clipping plane
 * @right: X position of the right clipping plane where it
 *   intersects the near clipping plane
 * @bottom: Y position of the bottom clipping plane where it
 *   intersects the near clipping plane
 * @top: Y position of the top clipping plane where it intersects
 *   the near clipping plane
 * @z_near: The distance to the near clipping plane (Must be positive)
 * @z_far: The distance to the far clipping plane (Must be positive)
 *
 * Replaces the current projection matrix with a perspective matrix
 * for a given viewing frustum defined by 4 side clip planes that
 * all cross through the origin and 2 near and far clip planes.
 *
 * Since: 1.10
 */
void
cogl_framebuffer_frustum (CoglFramebuffer *framebuffer,
                          float left,
                          float right,
                          float bottom,
                          float top,
                          float z_near,
                          float z_far);

#define cogl_framebuffer_orthographic cogl_framebuffer_orthographic_EXP
/**
 * cogl_framebuffer_orthographic:
 * @framebuffer: A #CoglFramebuffer pointer
 * @x_1: The x coordinate for the first vertical clipping plane
 * @y_1: The y coordinate for the first horizontal clipping plane
 * @x_2: The x coordinate for the second vertical clipping plane
 * @y_2: The y coordinate for the second horizontal clipping plane
 * @near: The <emphasis>distance</emphasis> to the near clipping
 *   plane (will be <emphasis>negative</emphasis> if the plane is
 *   behind the viewer)
 * @far: The <emphasis>distance</emphasis> to the far clipping
 *   plane (will be <emphasis>negative</emphasis> if the plane is
 *   behind the viewer)
 *
 * Replaces the current projection matrix with an orthographic projection
 * matrix.
 *
 * Since: 1.0
 */
void
cogl_framebuffer_orthographic (CoglFramebuffer *framebuffer,
                               float x_1,
                               float y_1,
                               float x_2,
                               float y_2,
                               float near,
                               float far);

#define cogl_framebuffer_get_projection_matrix \
  cogl_framebuffer_get_projection_matrix
/**
 * cogl_framebuffer_get_projection_matrix:
 * @framebuffer: A #CoglFramebuffer pointer
 * @matrix: (out): return location for the projection matrix
 *
 * Stores the current projection matrix in @matrix.
 */
void
cogl_framebuffer_get_projection_matrix (CoglFramebuffer *framebuffer,
                                        CoglMatrix *matrix);

#define cogl_framebuffer_set_projection_matrix \
  cogl_framebuffer_set_projection_matrix
/**
 * cogl_set_projection_matrix:
 * @framebuffer: A #CoglFramebuffer pointer
 * @matrix: the new projection matrix
 *
 * Sets @matrix as the new projection matrix.
 */
void
cogl_framebuffer_set_projection_matrix (CoglFramebuffer *framebuffer,
                                        CoglMatrix *matrix);

#define cogl_framebuffer_push_scissor_clip \
  cogl_framebuffer_push_scissor_clip_EXP
/**
 * cogl_framebuffer_push_scissor_clip:
 * @framebuffer: A #CoglFramebuffer pointer
 * @x: left edge of the clip rectangle in window coordinates
 * @y: top edge of the clip rectangle in window coordinates
 * @width: width of the clip rectangle
 * @height: height of the clip rectangle
 *
 * Specifies a rectangular clipping area for all subsequent drawing
 * operations. Any drawing commands that extend outside the rectangle
 * will be clipped so that only the portion inside the rectangle will
 * be displayed. The rectangle dimensions are not transformed by the
 * current model-view matrix.
 *
 * The rectangle is intersected with the current clip region. To undo
 * the effect of this function, call cogl_framebuffer_pop_clip().
 *
 * Since: 1.10
 * Stability: unstable
 */
void
cogl_framebuffer_push_scissor_clip (CoglFramebuffer *framebuffer,
                                    int x,
                                    int y,
                                    int width,
                                    int height);

#define cogl_framebuffer_push_rectangle_clip \
  cogl_framebuffer_push_rectangle_clip_EXP
/**
 * cogl_framebuffer_push_rectangle_clip:
 * @framebuffer: A #CoglFramebuffer pointer
 * @x_1: x coordinate for top left corner of the clip rectangle
 * @y_1: y coordinate for top left corner of the clip rectangle
 * @x_2: x coordinate for bottom right corner of the clip rectangle
 * @y_2: y coordinate for bottom right corner of the clip rectangle
 *
 * Specifies a modelview transformed rectangular clipping area for all
 * subsequent drawing operations. Any drawing commands that extend
 * outside the rectangle will be clipped so that only the portion
 * inside the rectangle will be displayed. The rectangle dimensions
 * are transformed by the current model-view matrix.
 *
 * The rectangle is intersected with the current clip region. To undo
 * the effect of this function, call cogl_framebuffer_pop_clip().
 *
 * Since: 1.10
 * Stability: unstable
 */
void
cogl_framebuffer_push_rectangle_clip (CoglFramebuffer *framebuffer,
                                      float x_1,
                                      float y_1,
                                      float x_2,
                                      float y_2);

#define cogl_framebuffer_push_path_clip \
  cogl_framebuffer_push_path_clip_EXP
/**
 * cogl_framebuffer_push_path_clip:
 * @framebuffer: A #CoglFramebuffer pointer
 * @path: The path to clip with.
 *
 * Sets a new clipping area using the silhouette of the specified,
 * filled @path.  The clipping area is intersected with the previous
 * clipping area. To restore the previous clipping area, call
 * cogl_framebuffer_pop_clip().
 *
 * Since: 1.0
 * Stability: unstable
 */
void
cogl_framebuffer_push_path_clip (CoglFramebuffer *framebuffer,
                                 CoglPath *path);

#define cogl_framebuffer_push_primitive_clip \
  cogl_framebuffer_push_primitive_clip_EXP
/**
 * cogl_framebuffer_push_primitive_clip:
 * @framebuffer: A #CoglFramebuffer pointer
 * @primitive: A #CoglPrimitive describing a flat 2D shape
 * @bounds_x1: x coordinate for the top-left corner of the primitives
 *             bounds
 * @bounds_y1: y coordinate for the top-left corner of the primitives
 *             bounds
 * @bounds_x2: x coordinate for the top-left corner of the primitives
 *             bounds
 * @bounds_y2: x coordinate for the bottom-right corner of the
 *             primitives bounds.
 * @bounds_x1: y coordinate for the bottom-right corner of the
 *             primitives bounds.
 *
 * Sets a new clipping area using a 2D shaped described with a
 * #CoglPrimitive. The shape must not contain self overlapping
 * geometry and must lie on a single 2D plane. A bounding box of the
 * 2D shape in local coordinates (the same coordinates used to
 * describe the shape) must be given. It is acceptable for the bounds
 * to be larger than the true bounds but behaviour is undefined if the
 * bounds are smaller than the true bounds.
 *
 * The primitive is transformed by the current model-view matrix and
 * the silhouette is intersected with the previous clipping area.  To
 * restore the previous clipping area, call
 * cogl_framebuffer_pop_clip().
 *
 * Since: 1.10
 * Stability: unstable
 */
void
cogl_framebuffer_push_primitive_clip (CoglFramebuffer *framebuffer,
                                      CoglPrimitive *primitive,
                                      float bounds_x1,
                                      float bounds_y1,
                                      float bounds_x2,
                                      float bounds_y2);

#define cogl_framebuffer_pop_clip cogl_framebuffer_pop_clip_EXP
/**
 * cogl_framebuffer_pop_clip:
 * @framebuffer: A #CoglFramebuffer pointer
 *
 * Reverts the clipping region to the state before the last call to
 * cogl_framebuffer_push_clip().
 *
 * Since: 1.10
 * Stability: unstable
 */
void
cogl_framebuffer_pop_clip (CoglFramebuffer *framebuffer);

/**
 * cogl_framebuffer_get_red_bits:
 * @framebuffer: a pointer to a #CoglFramebuffer
 *
 * Retrieves the number of red bits of @framebuffer
 *
 * Return value: the number of bits
 *
 * Since: 1.8
 * Stability: unstable
 */
#define cogl_framebuffer_get_red_bits cogl_framebuffer_get_red_bits_EXP
int
cogl_framebuffer_get_red_bits (CoglFramebuffer *framebuffer);

/**
 * cogl_framebuffer_get_green_bits:
 * @framebuffer: a pointer to a #CoglFramebuffer
 *
 * Retrieves the number of green bits of @framebuffer
 *
 * Return value: the number of bits
 *
 * Since: 1.8
 * Stability: unstable
 */
#define cogl_framebuffer_get_green_bits cogl_framebuffer_get_green_bits_EXP
int
cogl_framebuffer_get_green_bits (CoglFramebuffer *framebuffer);

/**
 * cogl_framebuffer_get_blue_bits:
 * @framebuffer: a pointer to a #CoglFramebuffer
 *
 * Retrieves the number of blue bits of @framebuffer
 *
 * Return value: the number of bits
 *
 * Since: 1.8
 * Stability: unstable
 */
#define cogl_framebuffer_get_blue_bits cogl_framebuffer_get_blue_bits_EXP
int
cogl_framebuffer_get_blue_bits (CoglFramebuffer *framebuffer);

/**
 * cogl_framebuffer_get_alpha_bits:
 * @framebuffer: a pointer to a #CoglFramebuffer
 *
 * Retrieves the number of alpha bits of @framebuffer
 *
 * Return value: the number of bits
 *
 * Since: 1.8
 * Stability: unstable
 */
#define cogl_framebuffer_get_alpha_bits cogl_framebuffer_get_alpha_bits_EXP
int
cogl_framebuffer_get_alpha_bits (CoglFramebuffer *framebuffer);

/**
 * cogl_framebuffer_get_dither_enabled:
 * @framebuffer: a pointer to a #CoglFramebuffer
 *
 * Returns whether dithering has been requested for the given @framebuffer.
 * See cogl_framebuffer_set_dither_enabled() for more details about dithering.
 *
 * <note>This may return %TRUE even when the underlying @framebuffer
 * display pipeline does not support dithering. This value only represents
 * the user's request for dithering.</note>
 *
 * Return value: %TRUE if dithering has been requested or %FALSE if not.
 */
gboolean
cogl_framebuffer_get_dither_enabled (CoglFramebuffer *framebuffer);

/**
 * cogl_framebuffer_set_dither_enabled:
 * @framebuffer: a pointer to a #CoglFramebuffer
 * @dither_enabled: %TRUE to enable dithering or %FALSE to disable
 *
 * Enables or disabled dithering if supported by the hardware.
 *
 * Dithering is a hardware dependent technique to increase the visible
 * color resolution beyond what the underlying hardware supports by playing
 * tricks with the colors placed into the framebuffer to give the illusion
 * of other colors. (For example this can be compared to half-toning used
 * by some news papers to show varying levels of grey even though their may
 * only be black and white are available).
 *
 * If the current display pipeline for @framebuffer does not support dithering
 * then this has no affect.
 *
 * Dithering is enabled by default.
 *
 * Since: 1.8
 * Stability: unstable
 */
void
cogl_framebuffer_set_dither_enabled (CoglFramebuffer *framebuffer,
                                     gboolean dither_enabled);

#define cogl_framebuffer_get_color_mask cogl_framebuffer_get_color_mask_EXP
/**
 * cogl_framebuffer_get_color_mask:
 * @framebuffer: a pointer to a #CoglFramebuffer
 *
 * Gets the current #CoglColorMask of which channels would be written to the
 * current framebuffer. Each bit set in the mask means that the
 * corresponding color would be written.
 *
 * Returns: A #CoglColorMask
 * Since: 1.8
 * Stability: unstable
 */
CoglColorMask
cogl_framebuffer_get_color_mask (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_set_color_mask cogl_framebuffer_set_color_mask_EXP
/**
 * cogl_framebuffer_set_color_mask:
 * @framebuffer: a pointer to a #CoglFramebuffer
 * @color_mask: A #CoglColorMask of which color channels to write to
 *              the current framebuffer.
 *
 * Defines a bit mask of which color channels should be written to the
 * given @framebuffer. If a bit is set in @color_mask that means that
 * color will be written.
 *
 * Since: 1.8
 * Stability: unstable
 */
void
cogl_framebuffer_set_color_mask (CoglFramebuffer *framebuffer,
                                 CoglColorMask color_mask);

#define cogl_framebuffer_get_color_format cogl_framebuffer_get_color_format_EXP
/**
 * cogl_framebuffer_get_color_format:
 * @framebuffer: A #CoglFramebuffer framebuffer
 *
 * Queries the common #CoglPixelFormat of all color buffers attached
 * to this framebuffer. For an offscreen framebuffer created with
 * cogl_offscreen_new_to_texture() this will correspond to the format
 * of the texture.
 *
 * Since: 1.8
 * Stability: unstable
 */
CoglPixelFormat
cogl_framebuffer_get_color_format (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_set_samples_per_pixel \
  cogl_framebuffer_set_samples_per_pixel_EXP
/**
 * cogl_framebuffer_set_samples_per_pixel:
 * @framebuffer: A #CoglFramebuffer framebuffer
 * @n: The minimum number of samples per pixel
 *
 * Requires that when rendering to @framebuffer then @n point samples
 * should be made per pixel which will all contribute to the final
 * resolved color for that pixel. The idea is that the hardware aims
 * to get quality similar to what you would get if you rendered
 * everything twice as big (for 4 samples per pixel) and then scaled
 * that image back down with filtering. It can effectively remove the
 * jagged edges of polygons and should be more efficient than if you
 * were to manually render at a higher resolution and downscale
 * because the hardware is often able to take some shortcuts. For
 * example the GPU may only calculate a single texture sample for all
 * points of a single pixel, and for tile based architectures all the
 * extra sample data (such as depth and stencil samples) may be
 * handled on-chip and so avoid increased demand on system memory
 * bandwidth.
 *
 * By default this value is usually set to 0 and that is referred to
 * as "single-sample" rendering. A value of 1 or greater is referred
 * to as "multisample" rendering.
 *
 * <note>There are some semantic differences between single-sample
 * rendering and multisampling with just 1 point sample such as it
 * being redundant to use the cogl_framebuffer_resolve_samples() and
 * cogl_framebuffer_resolve_samples_region() apis with single-sample
 * rendering.</note>
 *
 * <note>It's recommended that
 * cogl_framebuffer_resolve_samples_region() be explicitly used at the
 * end of rendering to a point sample buffer to minimize the number of
 * samples that get resolved. By default Cogl will implicitly resolve
 * all framebuffer samples but if only a small region of a
 * framebuffer has changed this can lead to redundant work being
 * done.</note>
 *
 * Since: 1.8
 * Stability: unstable
 */
void
cogl_framebuffer_set_samples_per_pixel (CoglFramebuffer *framebuffer,
                                        int samples_per_pixel);

#define cogl_framebuffer_get_samples_per_pixel \
  cogl_framebuffer_get_samples_per_pixel_EXP
/**
 * cogl_framebuffer_get_samples_per_pixel:
 * @framebuffer: A #CoglFramebuffer framebuffer
 *
 * Gets the number of points that are sampled per-pixel when
 * rasterizing geometry. Usually by default this will return 0 which
 * means that single-sample not multisample rendering has been chosen.
 * When using a GPU supporting multisample rendering it's possible to
 * increase the number of samples per pixel using
 * cogl_framebuffer_set_samples_per_pixel().
 *
 * Calling cogl_framebuffer_get_samples_per_pixel() before the
 * framebuffer has been allocated will simply return the value set
 * using cogl_framebuffer_set_samples_per_pixel(). After the
 * framebuffer has been allocated the value will reflect the actual
 * number of samples that will be made by the GPU.
 *
 * Returns: The number of point samples made per pixel when
 *          rasterizing geometry or 0 if single-sample rendering
 *          has been chosen.
 *
 * Since: 1.10
 * Stability: unstable
 */
int
cogl_framebuffer_get_samples_per_pixel (CoglFramebuffer *framebuffer);


#define cogl_framebuffer_resolve_samples \
  cogl_framebuffer_resolve_samples_EXP
/**
 * cogl_framebuffer_resolve_samples:
 * @framebuffer: A #CoglFramebuffer framebuffer
 *
 * When point sample rendering (also known as multisample rendering)
 * has been enabled via cogl_framebuffer_set_samples_per_pixel()
 * then you can optionally call this function (or
 * cogl_framebuffer_resolve_samples_region()) to explicitly resolve
 * the point samples into values for the final color buffer.
 *
 * Some GPUs will implicitly resolve the point samples during
 * rendering and so this function is effectively a nop, but with other
 * architectures it is desirable to defer the resolve step until the
 * end of the frame.
 *
 * Since Cogl will automatically ensure samples are resolved if the
 * target color buffer is used as a source this API only needs to be
 * used if explicit control is desired - perhaps because you want to
 * ensure that the resolve is completed in advance to avoid later
 * having to wait for the resolve to complete.
 *
 * If you are performing incremental updates to a framebuffer you
 * should consider using cogl_framebuffer_resolve_samples_region()
 * instead to avoid resolving redundant pixels.
 *
 * Since: 1.8
 * Stability: unstable
 */
void
cogl_framebuffer_resolve_samples (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_resolve_samples_region \
  cogl_framebuffer_resolve_samples_region_EXP
/**
 * cogl_framebuffer_resolve_samples_region:
 * @framebuffer: A #CoglFramebuffer framebuffer
 * @x: top-left x coordinate of region to resolve
 * @y: top-left y coordinate of region to resolve
 * @width: width of region to resolve
 * @height: height of region to resolve
 *
 * When point sample rendering (also known as multisample rendering)
 * has been enabled via cogl_framebuffer_set_samples_per_pixel()
 * then you can optionally call this function (or
 * cogl_framebuffer_resolve_samples()) to explicitly resolve the point
 * samples into values for the final color buffer.
 *
 * Some GPUs will implicitly resolve the point samples during
 * rendering and so this function is effectively a nop, but with other
 * architectures it is desirable to defer the resolve step until the
 * end of the frame.
 *
 * Use of this API is recommended if incremental, small updates to
 * a framebuffer are being made because by default Cogl will
 * implicitly resolve all the point samples of the framebuffer which
 * can result in redundant work if only a small number of samples have
 * changed.
 *
 * Because some GPUs implicitly resolve point samples this function
 * only guarantees that at-least the region specified will be resolved
 * and if you have rendered to a larger region then it's possible that
 * other samples may be implicitly resolved.
 *
 * Since: 1.8
 * Stability: unstable
 */
void
cogl_framebuffer_resolve_samples_region (CoglFramebuffer *framebuffer,
                                         int x,
                                         int y,
                                         int width,
                                         int height);

#define cogl_framebuffer_get_context cogl_framebuffer_get_context_EXP
/**
 * @framebuffer: A #CoglFramebuffer
 *
 * Can be used to query the #CoglContext a given @framebuffer was
 * instantiated within. This is the #CoglContext that was passed to
 * cogl_onscreen_new() for example.
 *
 * Return value: The #CoglContext that the given @framebuffer was
 *               instantiated within.
 * Since: 1.8
 * Stability: unstable
 */
CoglContext *
cogl_framebuffer_get_context (CoglFramebuffer *framebuffer);

#define cogl_framebuffer_clear cogl_framebuffer_clear_EXP
/**
 * cogl_framebuffer_clear:
 * @framebuffer: A #CoglFramebuffer
 * @buffers: A mask of #CoglBufferBit<!-- -->'s identifying which auxiliary
 *   buffers to clear
 * @color: The color to clear the color buffer too if specified in
 *         @buffers.
 *
 * Clears all the auxiliary buffers identified in the @buffers mask, and if
 * that includes the color buffer then the specified @color is used.
 *
 * Since: 1.8
 * Stability: unstable
 */
void
cogl_framebuffer_clear (CoglFramebuffer *framebuffer,
                        unsigned long buffers,
                        const CoglColor *color);

#define cogl_framebuffer_clear4f cogl_framebuffer_clear4f_EXP
/**
 * cogl_framebuffer_clear4f:
 * @framebuffer: A #CoglFramebuffer
 * @buffers: A mask of #CoglBufferBit<!-- -->'s identifying which auxiliary
 *   buffers to clear
 * @red: The red component of color to clear the color buffer too if
 *       specified in @buffers.
 * @green: The green component of color to clear the color buffer too if
 *         specified in @buffers.
 * @blue: The blue component of color to clear the color buffer too if
 *        specified in @buffers.
 * @alpha: The alpha component of color to clear the color buffer too if
 *         specified in @buffers.
 *
 * Clears all the auxiliary buffers identified in the @buffers mask, and if
 * that includes the color buffer then the specified @color is used.
 *
 * Since: 1.8
 * Stability: unstable
 */
void
cogl_framebuffer_clear4f (CoglFramebuffer *framebuffer,
                          unsigned long buffers,
                          float red,
                          float green,
                          float blue,
                          float alpha);

/* XXX: Should we take an n_buffers + buffer id array instead of using
 * the CoglBufferBits type which doesn't seem future proof? */
#define cogl_framebuffer_discard_buffers cogl_framebuffer_discard_buffers_EXP
/**
 * cogl_framebuffer_discard_buffers:
 * @framebuffer: A #CoglFramebuffer
 * @buffers: A #CoglBufferBit mask of which ancillary buffers you want
 *           to discard.
 *
 * Declares that the specified @buffers no longer need to be referenced
 * by any further rendering commands. This can be an important
 * optimization to avoid subsequent frames of rendering depending on
 * the results of a previous frame.
 *
 * For example; some tile-based rendering GPUs are able to avoid allocating and
 * accessing system memory for the depth and stencil buffer so long as these
 * buffers are not required as input for subsequent frames and that can save a
 * significant amount of memory bandwidth used to save and restore their
 * contents to system memory between frames.
 *
 * It is currently considered an error to try and explicitly discard the color
 * buffer by passing %COGL_BUFFER_BIT_COLOR. This is because the color buffer is
 * already implicitly discard when you finish rendering to a #CoglOnscreen
 * framebuffer, and it's not meaningful to try and discard the color buffer of
 * a #CoglOffscreen framebuffer since they are single-buffered.
 *
 *
 * Since: 1.8
 * Stability: unstable
 */
void
cogl_framebuffer_discard_buffers (CoglFramebuffer *framebuffer,
                                  unsigned long buffers);

/**
 * cogl_framebuffer_finish:
 * @framebuffer: A #CoglFramebuffer pointer
 *
 * This blocks the CPU until all pending rendering associated with the
 * specified framebuffer has completed. It's very rare that developers should
 * ever need this level of synchronization with the GPU and should never be
 * used unless you clearly understand why you need to explicitly force
 * synchronization.
 *
 * One example might be for benchmarking purposes to be sure timing
 * measurements reflect the time that the GPU is busy for not just the time it
 * takes to queue rendering commands.
 *
 * Stability: unstable
 * Since: 1.10
 */
void
cogl_framebuffer_finish (CoglFramebuffer *framebuffer);

#define cogl_get_draw_framebuffer cogl_get_draw_framebuffer_EXP
CoglFramebuffer *
cogl_get_draw_framebuffer (void);

#endif /* COGL_ENABLE_EXPERIMENTAL_API */

/* XXX: Note these are defined outside the COGL_ENABLE_EXPERIMENTAL_API guard since
 * otherwise the glib-mkenums stuff will get upset. */

#define cogl_framebuffer_error_quark cogl_framebuffer_error_quark_EXP
GQuark
cogl_framebuffer_error_quark (void);

#define COGL_FRAMEBUFFER_ERROR (cogl_framebuffer_error_quark ())

typedef enum { /*< prefix=COGL_FRAMEBUFFER_ERROR >*/
  COGL_FRAMEBUFFER_ERROR_ALLOCATE
} CoglFramebufferError;

G_END_DECLS

#endif /* __COGL_FRAMEBUFFER_H */

