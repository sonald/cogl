/* This file has been generated by parse-gir.py, do not hand edit */
using System;
using System.Runtime.InteropServices;

namespace Cogl
{
    public partial class Texture
    {
        [DllImport("cogl2.dll")]
        public static extern PixelFormat cogl_texture_get_format(IntPtr o);

        public PixelFormat GetFormat()
        {
            return cogl_texture_get_format(handle);
        }

        [DllImport("cogl2.dll")]
        public static extern int cogl_texture_get_max_waste(IntPtr o);

        public int GetMaxWaste()
        {
            return cogl_texture_get_max_waste(handle);
        }

    }
}
