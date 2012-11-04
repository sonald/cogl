/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2012 Intel Corporation.
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
 * Authors:
 *   Damien Lespiau <damien.lespiau@intel.com>
 */

using System;
using System.Runtime.InteropServices;

namespace Cogl
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Color
    {
        public float red;
        public float green;
        public float blue;
        public float alpha;

        public Color(float r, float g, float b, float a)
        {
            red = r;
            green = g;
            blue = b;
            alpha = a;
        }

        public Color(byte r, byte g, byte b, byte a)
        {
            red = r / 255.0f;
            green = g / 255.0f;
            blue = b / 255.0f;
            alpha = a / 255.0f;
        }

        [DllImport("cogl2.dll")]
        private static extern void cogl_color_premultiply(ref Color color);

        public void Premultiply()
        {
            cogl_color_premultiply(ref this);
        }

        [DllImport("cogl2.dll")]
        private static extern void cogl_color_unpremultiply(ref Color color);

        public void UnPremultiply()
        {
            cogl_color_unpremultiply(ref this);
        }

        public override String ToString()
        {
            return String.Format("({0},{1},{2},{3})", red, green, blue, alpha);
        }
    }
}