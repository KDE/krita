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

#define TILE_SIZE 64
#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

MyPaintSurface::MyPaintSurface(KisPaintDeviceSP src, KisPaintDeviceSP dst)
    : m_src(src)
    , m_dst(dst)
{
    m_rgb16 = KoColorSpaceRegistry::instance()->rgb16();
    // fake a mypaint tile
    m_dstData = m_dst->colorSpace()->allocPixelBuffer(TILE_SIZE * TILE_SIZE, true);
    m_dstRgb16Data = m_rgb16->allocPixelBuffer(TILE_SIZE * TILE_SIZE, true);
}

bool MyPaintSurface::draw_dab (float x, float y,
                               float radius,
                               float color_r, float color_g, float color_b,
                               float opaque, float hardness,
                               float eraser_target_alpha,
                               float aspect_ratio, float angle)
{
    if (aspect_ratio<1.0) aspect_ratio=1.0;

    float r_fringe;
    int xp, yp;
    float xx, yy, rr;
    float one_over_radius2;

    eraser_target_alpha = CLAMP(eraser_target_alpha, 0.0, 1.0);
    quint32 color_r_ = color_r * (1<<15);
    quint32 color_g_ = color_g * (1<<15);
    quint32 color_b_ = color_b * (1<<15);
    color_r = CLAMP(color_r, 0, (1<<15));
    color_g = CLAMP(color_g, 0, (1<<15));
    color_b = CLAMP(color_b, 0, (1<<15));

    opaque = CLAMP(opaque, 0.0, 1.0);
    hardness = CLAMP(hardness, 0.0, 1.0);
    if (opaque == 0.0) return false;
    if (radius < 0.1) return false;
    if (hardness == 0.0) return false; // infintly small point, rest transparent

    r_fringe = radius + 1;
    rr = radius*radius;
    one_over_radius2 = 1.0/rr;

    int tx1 = floor(floor(x - r_fringe) / TILE_SIZE);
    int tx2 = floor(floor(x + r_fringe) / TILE_SIZE);
    int ty1 = floor(floor(y - r_fringe) / TILE_SIZE);
    int ty2 = floor(floor(y + r_fringe) / TILE_SIZE);
    int tx, ty;
    for (ty = ty1; ty <= ty2; ty++) {
        for (tx = tx1; tx <= tx2; tx++) {

            //uint16_t * rgba_p = get_tile_memory(tx, ty, false);
            m_dst->readBytes(m_dstData, tx, ty, TILE_SIZE, TILE_SIZE);
            m_dst->colorSpace()->convertPixelsTo(m_dstData, m_dstRgb16Data, m_rgb16, TILE_SIZE * TILE_SIZE);

            quint16* rgba_p = reinterpret_cast<quint16*>(m_dstRgb16Data);
            if (!rgba_p) {
                printf("Python exception during draw_dab()!\n");
                return true;
            }

            float xc = x - tx*TILE_SIZE;
            float yc = y - ty*TILE_SIZE;

            int x0 = floor (xc - r_fringe);
            int y0 = floor (yc - r_fringe);
            int x1 = ceil (xc + r_fringe);
            int y1 = ceil (yc + r_fringe);
            if (x0 < 0) x0 = 0;
            if (y0 < 0) y0 = 0;
            if (x1 > TILE_SIZE-1) x1 = TILE_SIZE-1;
            if (y1 > TILE_SIZE-1) y1 = TILE_SIZE-1;

            float angle_rad=angle/360*2*M_PI;
            float cs=cos(angle_rad);
            float sn=sin(angle_rad);

            for (yp = y0; yp <= y1; yp++) {
                yy = (yp + 0.5 - yc);
                for (xp = x0; xp <= x1; xp++) {
                    xx = (xp + 0.5 - xc);
                    // code duplication, see brush::count_dabs_to()
                    float yyr=(yy*cs-xx*sn)*aspect_ratio;
                    float xxr=yy*sn+xx*cs;
                    rr = (yyr*yyr + xxr*xxr) * one_over_radius2;
                    // rr is in range 0.0..1.0*sqrt(2)

                    if (rr <= 1.0) {
                        float opa = opaque;
                        if (hardness < 1.0) {
                            if (rr < hardness) {
                                opa *= rr + 1-(rr/hardness);
                                // hardness == 0 is nonsense, excluded above
                            } else {
                                opa *= hardness/(1-hardness)*(1-rr);
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
                        // OPTIMIZE: separate function for the standard case without erasing?
                        // OPTIMIZE: don't use floats here in the inner loop?

                        quint32 opa_a = (1<<15)*opa;   // topAlpha
                        quint32 opa_b = (1<<15)-opa_a; // bottomAlpha

                        // only for eraser, or for painting with translucent-making colors
                        opa_a *= eraser_target_alpha;

                        int idx = (yp*TILE_SIZE + xp)*4;
                        rgba_p[idx+3] = opa_a + (opa_b*rgba_p[idx+3])/(1<<15);
                        rgba_p[idx+0] = (opa_a*color_r_ + opa_b*rgba_p[idx+0])/(1<<15);
                        rgba_p[idx+1] = (opa_a*color_g_ + opa_b*rgba_p[idx+1])/(1<<15);
                        rgba_p[idx+2] = (opa_a*color_b_ + opa_b*rgba_p[idx+2])/(1<<15);
                    }
                }
            }
            m_rgb16->convertPixelsTo(m_dstRgb16Data, m_dstData, m_dst->colorSpace(), TILE_SIZE * TILE_SIZE);
            m_dst->writeBytes(m_dstData, tx, ty, TILE_SIZE, TILE_SIZE);
        }
    }


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
