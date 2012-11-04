/* This file has been generated by parse-gir.py, do not hand edit */
using System;
using System.Runtime.InteropServices;

namespace Cogl
{
    public partial class Pipeline
    {
        [DllImport("cogl2.dll")]
        public static extern IntPtr cogl_pipeline_copy(IntPtr o);

        public Pipeline Copy()
        {
            IntPtr p = cogl_pipeline_copy(handle);
            return new Pipeline(p);
        }

        [DllImport("cogl2.dll")]
        public static extern PipelineAlphaFunc cogl_pipeline_get_alpha_test_function(IntPtr o);

        public PipelineAlphaFunc GetAlphaTestFunction()
        {
            return cogl_pipeline_get_alpha_test_function(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern float cogl_pipeline_get_alpha_test_reference(IntPtr o);

        public float GetAlphaTestReference()
        {
            return cogl_pipeline_get_alpha_test_reference(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_get_color(IntPtr o);

        public void GetColor()
        {
            cogl_pipeline_get_color(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern ColorMask cogl_pipeline_get_color_mask(IntPtr o);

        public ColorMask GetColorMask()
        {
            return cogl_pipeline_get_color_mask(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern PipelineCullFaceMode cogl_pipeline_get_cull_face_mode(IntPtr o);

        public PipelineCullFaceMode GetCullFaceMode()
        {
            return cogl_pipeline_get_cull_face_mode(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern Winding cogl_pipeline_get_front_face_winding(IntPtr o);

        public Winding GetFrontFaceWinding()
        {
            return cogl_pipeline_get_front_face_winding(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern PipelineFilter cogl_pipeline_get_layer_mag_filter(IntPtr o, int layer_index);

        public PipelineFilter GetLayerMagFilter(int layer_index)
        {
            return cogl_pipeline_get_layer_mag_filter(handle, layer_index);
        }

        [DllImport("cogl2.dll")]
        public static extern PipelineFilter cogl_pipeline_get_layer_min_filter(IntPtr o, int layer_index);

        public PipelineFilter GetLayerMinFilter(int layer_index)
        {
            return cogl_pipeline_get_layer_min_filter(handle, layer_index);
        }

        [DllImport("cogl2.dll")]
        public static extern bool cogl_pipeline_get_layer_point_sprite_coords_enabled(IntPtr o, int layer_index);

        public bool GetLayerPointSpriteCoordsEnabled(int layer_index)
        {
            return cogl_pipeline_get_layer_point_sprite_coords_enabled(handle, layer_index);
        }

        [DllImport("cogl2.dll")]
        public static extern IntPtr cogl_pipeline_get_layer_texture(IntPtr o, int layer_index);

        public Texture GetLayerTexture(int layer_index)
        {
            IntPtr p = cogl_pipeline_get_layer_texture(handle, layer_index);
            return new Texture(p);
        }

        [DllImport("cogl2.dll")]
        public static extern PipelineWrapMode cogl_pipeline_get_layer_wrap_mode_p(IntPtr o, int layer_index);

        public PipelineWrapMode GetLayerWrapModeP(int layer_index)
        {
            return cogl_pipeline_get_layer_wrap_mode_p(handle, layer_index);
        }

        [DllImport("cogl2.dll")]
        public static extern PipelineWrapMode cogl_pipeline_get_layer_wrap_mode_s(IntPtr o, int layer_index);

        public PipelineWrapMode GetLayerWrapModeS(int layer_index)
        {
            return cogl_pipeline_get_layer_wrap_mode_s(handle, layer_index);
        }

        [DllImport("cogl2.dll")]
        public static extern PipelineWrapMode cogl_pipeline_get_layer_wrap_mode_t(IntPtr o, int layer_index);

        public PipelineWrapMode GetLayerWrapModeT(int layer_index)
        {
            return cogl_pipeline_get_layer_wrap_mode_t(handle, layer_index);
        }

        [DllImport("cogl2.dll")]
        public static extern int cogl_pipeline_get_n_layers(IntPtr o);

        public int GetNLayers()
        {
            return cogl_pipeline_get_n_layers(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern float cogl_pipeline_get_point_size(IntPtr o);

        public float GetPointSize()
        {
            return cogl_pipeline_get_point_size(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_remove_layer(IntPtr o, int layer_index);

        public void RemoveLayer(int layer_index)
        {
            cogl_pipeline_remove_layer(handle, layer_index);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_alpha_test_function(IntPtr o, PipelineAlphaFunc alpha_func, float alpha_reference);

        public void SetAlphaTestFunction(PipelineAlphaFunc alpha_func, float alpha_reference)
        {
            cogl_pipeline_set_alpha_test_function(handle, alpha_func, alpha_reference);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_blend_constant(IntPtr o, ref Color constant_color);

        public void SetBlendConstant(ref Color constant_color)
        {
            cogl_pipeline_set_blend_constant(handle, ref constant_color);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_color(IntPtr o, ref Color color);

        public void SetColor(ref Color color)
        {
            cogl_pipeline_set_color(handle, ref color);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_color4f(IntPtr o, float red, float green, float blue, float alpha);

        public void SetColor(float red, float green, float blue, float alpha)
        {
            cogl_pipeline_set_color4f(handle, red, green, blue, alpha);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_color_mask(IntPtr o, ColorMask color_mask);

        public void SetColorMask(ColorMask color_mask)
        {
            cogl_pipeline_set_color_mask(handle, color_mask);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_cull_face_mode(IntPtr o, PipelineCullFaceMode cull_face_mode);

        public void SetCullFaceMode(PipelineCullFaceMode cull_face_mode)
        {
            cogl_pipeline_set_cull_face_mode(handle, cull_face_mode);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_front_face_winding(IntPtr o, Winding front_winding);

        public void SetFrontFaceWinding(Winding front_winding)
        {
            cogl_pipeline_set_front_face_winding(handle, front_winding);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_layer_combine_constant(IntPtr o, int layer_index, ref Color constant);

        public void SetLayerCombineConstant(int layer_index, ref Color constant)
        {
            cogl_pipeline_set_layer_combine_constant(handle, layer_index, ref constant);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_layer_filters(IntPtr o, int layer_index, PipelineFilter min_filter, PipelineFilter mag_filter);

        public void SetLayerFilters(int layer_index, PipelineFilter min_filter, PipelineFilter mag_filter)
        {
            cogl_pipeline_set_layer_filters(handle, layer_index, min_filter, mag_filter);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_layer_matrix(IntPtr o, int layer_index, ref Matrix matrix);

        public void SetLayerMatrix(int layer_index, ref Matrix matrix)
        {
            cogl_pipeline_set_layer_matrix(handle, layer_index, ref matrix);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_layer_null_texture(IntPtr o, int layer_index, TextureType texure_type);

        public void SetLayerNullTexture(int layer_index, TextureType texure_type)
        {
            cogl_pipeline_set_layer_null_texture(handle, layer_index, texure_type);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_layer_texture(IntPtr o, int layer_index, IntPtr texture);

        public void SetLayerTexture(int layer_index, Texture texture)
        {
            cogl_pipeline_set_layer_texture(handle, layer_index, texture.Handle);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_layer_wrap_mode(IntPtr o, int layer_index, PipelineWrapMode mode);

        public void SetLayerWrapMode(int layer_index, PipelineWrapMode mode)
        {
            cogl_pipeline_set_layer_wrap_mode(handle, layer_index, mode);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_layer_wrap_mode_p(IntPtr o, int layer_index, PipelineWrapMode mode);

        public void SetLayerWrapModeP(int layer_index, PipelineWrapMode mode)
        {
            cogl_pipeline_set_layer_wrap_mode_p(handle, layer_index, mode);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_layer_wrap_mode_s(IntPtr o, int layer_index, PipelineWrapMode mode);

        public void SetLayerWrapModeS(int layer_index, PipelineWrapMode mode)
        {
            cogl_pipeline_set_layer_wrap_mode_s(handle, layer_index, mode);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_layer_wrap_mode_t(IntPtr o, int layer_index, PipelineWrapMode mode);

        public void SetLayerWrapModeT(int layer_index, PipelineWrapMode mode)
        {
            cogl_pipeline_set_layer_wrap_mode_t(handle, layer_index, mode);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_point_size(IntPtr o, float point_size);

        public void SetPointSize(float point_size)
        {
            cogl_pipeline_set_point_size(handle, point_size);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_uniform_1f(IntPtr o, int uniform_location, float value);

        public void SetUniform1f(int uniform_location, float value)
        {
            cogl_pipeline_set_uniform_1f(handle, uniform_location, value);
        }

        [DllImport("cogl2.dll")]
        public static extern void cogl_pipeline_set_uniform_1i(IntPtr o, int uniform_location, int value);

        public void SetUniform1i(int uniform_location, int value)
        {
            cogl_pipeline_set_uniform_1i(handle, uniform_location, value);
        }

    }
}