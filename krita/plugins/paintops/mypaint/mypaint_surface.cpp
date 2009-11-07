/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_paint_device.h"
#include "mypaint_surface.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

MyPaintSurface::MyPaintSurface(KisPaintDeviceSP src, KisPaintDeviceSP dst)
    : m_src(src)
    , m_dst(dst)
{
    m_color = KoColor(dst->colorSpace());
}

bool MyPaintSurface::draw_dab (float x, float y,
                               float radius,
                               float color_r, float color_g, float color_b,
                               float opaque, float hardness,
                               float alpha_eraser,
                               float aspect_ratio, float angle)
{

    qDebug() << "x" << x << "y" << y << "radius" << radius
             << "color_r" << color_r << "color_g" << color_g << "color_b" << color_b
             << "opaque" << opaque << "hardness" << hardness
             << "alpha_eraser" << alpha_eraser
             << "aspect_ratio" << aspect_ratio << "angle" << angle;

    if (aspect_ratio < 1.0) aspect_ratio=1.0;

    alpha_eraser = CLAMP(alpha_eraser, 0.0, 1.0);
    opaque = CLAMP(opaque, 0.0, 1.0);
    hardness = CLAMP(hardness, 0.0, 1.0);

    if (opaque == 0.0) return false;
    if (radius < 0.1) return false;
    if (hardness == 0.0) return false; // infintly small point, rest transparent

    float r_fringe = radius + 1;
    float rr = radius*radius;
    float one_over_radius2 = 1.0/rr;

    m_qcolor.setRgbF(color_r, color_g, color_b, opaque);
    m_color.fromQColor(m_qcolor);


    return true;

}

void MyPaintSurface::get_color (float x, float y,
                                float radius,
                                float* color_r, float* color_g, float* color_b, float* color_a )
{
    // XXX: calculate the average color inside this anti-aliased circle...
}
