/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2008 by Martin Renold <martinxyz@gmx.ch>
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

#include <math.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_random_accessor.h>

#include "mypaint_surface.h"

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

MyPaintSurface::MyPaintSurface(KisPaintDeviceSP src, KisPaintDeviceSP dst)
    : m_src(src)
    , m_dst(dst)
{
    m_rgb16 = KoColorSpaceRegistry::instance()->rgb16();
}

bool MyPaintSurface::draw_dab (float x, float y,
                               float radius,
                               float color_r, float color_g, float color_b,
                               float opaque, float hardness,
                               float eraser_target_alpha,
                               float aspect_ratio, float angle)
{

//    qDebug() << "x" << x << "y" << y << "radius" << radius
//            << "color_r" << color_r << "color_g" << color_g << "color_b" << color_b
//            << "opaque" << opaque << "hardness" << hardness
//            << "eraser_target_alpha" << eraser_target_alpha
//            << "aspect_ratio" << aspect_ratio << "angle" << angle;

    if (aspect_ratio < 1.0) aspect_ratio = 1.0;

    eraser_target_alpha = CLAMP(eraser_target_alpha, 0.0, 1.0);

    quint16 color_r_ = color_r * (1<<15);
    quint16 color_g_ = color_g * (1<<15);
    quint16 color_b_ = color_b * (1<<15);
    color_r = CLAMP(color_r, 0, (1<<15));
    color_g = CLAMP(color_g, 0, (1<<15));
    color_b = CLAMP(color_b, 0, (1<<15));

    opaque = CLAMP(opaque, 0.0, 1.0);
    hardness = CLAMP(hardness, 0.0, 1.0);

    if (opaque == 0.0) return false;
    if (radius < 0.1) return false;
    if (hardness == 0.0) return false; // infintly small point, rest transparent

    float r_fringe = radius + 1;
    float rr = radius * radius;
    float one_over_radius2 = 1.0/rr;

    // the square in which the round mypaint dab fits
    int x1 = floor(floor(x - r_fringe));
    int x2 = floor(floor(x + r_fringe));
    int y1 = floor(floor(y - r_fringe));
    int y2 = floor(floor(y + r_fringe));

    quint8* dstData = m_dst->colorSpace()->allocPixelBuffer(x2 - x1);
    quint8* rgb16Data = KoColorSpaceRegistry::instance()->rgb16()->allocPixelBuffer(x2 - x1);

    // figure out the area the circular dab covers within the square
    float angle_rad = angle / 360 * 2 * M_PI;
    float cs = cos(angle_rad);
    float sn = sin(angle_rad);

    for (int row = y1; row < y2; ++row) {

        m_dst->readBytes(dstData, QRect(x1, row, x2 - x1, 1));
        m_dst->colorSpace()->convertPixelsTo(dstData, rgb16Data, m_rgb16, x2 - x1);

        for (int col = x1; col < x2; ++col) {

            float yyr = (row * cs - col * sn) * aspect_ratio;
            float xxr = (row * sn + col * sn);

            // rr is in range 0.0..1.0*sqrt(2)
            rr = (yyr * yyr + xxr * xxr) * one_over_radius2;
            qDebug() << rr;
            if (rr <= 1.0) {

                float opa = opaque;
                if (hardness < 1.0) {
                    if (rr < hardness) {
                        opa *= rr + 1 - (rr/hardness);
                        // hardness == 0 is nonsense, excluded above
                    } else {
                        opa *= hardness / (1 - hardness) * (1 - rr);
                    }
                }
                // We are manipulating pixels with premultiplied alpha directly.
                // This is an "over" operation (opa = topAlpha).
                // In the formula below, topColor is assumed to be premultiplied.
                //
                //               opa_a      <   opa_b      >
                // resultAlpha = topAlpha + (1.0 - topAlpha) * bottomAlpha
                // resultColor = topColor + (1.0 - topAlpha) * bottomColor
                //
                // (at least for the normal case where eraser_target_alpha == 1.0)

                quint32 opa_a = (1<<15)*opa;   // topAlpha
                quint32 opa_b = (1<<15)-opa_a; // bottomAlpha

                // only for eraser, or for painting with translucent-making colors
                opa_a *= eraser_target_alpha;

                int idx = (x2 - x1) * 4;

                // we have bgra, mypaint has rgba
                dstData[idx+3] =  opa_a +           (opa_b * dstData[idx+3]) / (1<<15);
                dstData[idx+2] = (opa_a * color_r_ + opa_b * dstData[idx+2]) / (1<<15);
                dstData[idx+1] = (opa_a * color_g_ + opa_b * dstData[idx+1]) / (1<<15);
                dstData[idx+1] = (opa_a * color_b_ + opa_b * dstData[idx+1]) / (1<<15);
            }
        }
        m_rgb16->convertPixelsTo(rgb16Data, dstData, m_dst->colorSpace(), x2 - x1);
        m_dst->writeBytes(dstData, QRect(x1, row, x2 - x1, 1));
    }

    delete[] dstData;
    delete[] rgb16Data;

    return true;

}

void MyPaintSurface::get_color (float x, float y,
                                float radius,
                                float* color_r, float* color_g, float* color_b, float* color_a )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(radius);
    Q_UNUSED(color_r);
    Q_UNUSED(color_g);
    Q_UNUSED(color_b);
    Q_UNUSED(color_a);
    // XXX: calculate the average color inside this anti-aliased circle.

    // for every pixel in the square, get the color and calculate the weight

    // let pigment calculate the average color

    // convert to 16 bit rgba
}
