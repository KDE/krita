/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <limits.h>

#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>

#include "kis_image.h"
#include "kis_strategy_colorspace_cmyk.h"
#include "tiles/kispixeldata.h"

namespace {
    const Q_INT32 MAX_CHANNEL_CMYK = 4;
    // Is it actually possible to have transparency with CMYK?
    const Q_INT32 MAX_CHANNEL_CMYKA = 5;
}

// Init static data
ColorLUT KisStrategyColorSpaceCMYK::m_rgbLUT = ColorLUT();


KisStrategyColorSpaceCMYK::KisStrategyColorSpaceCMYK() : m_pixmap(RENDER_WIDTH * 2, RENDER_HEIGHT * 2)
{
    m_buf = new QUANTUM[RENDER_WIDTH * RENDER_HEIGHT * MAX_CHANNEL_CMYKA];
}

KisStrategyColorSpaceCMYK::~KisStrategyColorSpaceCMYK()
{
    delete[] m_buf;
}

void KisStrategyColorSpaceCMYK::nativeColor(const KoColor& c, QUANTUM *dst)
{
    dst[PIXEL_CYAN] = upscale( c.C() );
    dst[PIXEL_MAGENTA] = upscale( c.M() );
    dst[PIXEL_YELLOW] = upscale( c.Y() );
    dst[PIXEL_BLACK] = upscale( c.K() );
}

void KisStrategyColorSpaceCMYK::nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst)
{
    dst[PIXEL_CYAN] = upscale( c.C() );
    dst[PIXEL_MAGENTA] = upscale( c.M() );
    dst[PIXEL_YELLOW] = upscale( c.Y() );
    dst[PIXEL_BLACK] = upscale( c.K() );
    dst[PIXEL_CMYK_ALPHA] = opacity;
}

void KisStrategyColorSpaceCMYK::nativeColor(const QColor& c, QUANTUM *dst)
{
    KoColor k = KoColor( c );

    dst[PIXEL_CYAN] = upscale( k.C() );
    dst[PIXEL_MAGENTA] = upscale( k.M() );
    dst[PIXEL_YELLOW] = upscale( k.Y() );
    dst[PIXEL_BLACK] = upscale( k.K() );
}

void KisStrategyColorSpaceCMYK::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst)
{
    KoColor k = KoColor( c );

    dst[PIXEL_CYAN] = upscale( k.C() );
    dst[PIXEL_MAGENTA] = upscale( k.M() );
    dst[PIXEL_YELLOW] = upscale( k.Y() );
    dst[PIXEL_BLACK] = upscale( k.K() );
    dst[PIXEL_CMYK_ALPHA] = opacity;

}

void KisStrategyColorSpaceCMYK::nativeColor(QRgb rgb, QUANTUM *dst)
{
    KoColor k = KoColor(QColor( rgb ));

    dst[PIXEL_CYAN] = upscale( k.C() );
    dst[PIXEL_MAGENTA] = upscale( k.M() );
    dst[PIXEL_YELLOW] = upscale( k.Y() );
    dst[PIXEL_BLACK] = upscale( k.K() );
}

void KisStrategyColorSpaceCMYK::nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst)
{
    KoColor k = KoColor(QColor( rgb ));

    dst[PIXEL_CYAN] = upscale( k.C() );
    dst[PIXEL_MAGENTA] = upscale( k.M() );
    dst[PIXEL_YELLOW] = upscale( k.Y() );
    dst[PIXEL_BLACK] = upscale( k.K() );
    dst[PIXEL_CMYK_ALPHA] = opacity;
}

void KisStrategyColorSpaceCMYK::composite(QUANTUM *dst, QUANTUM *src, Q_INT32 opacity, CompositeOp op) const
{
    QUANTUM alpha;
    QUANTUM invAlpha;
    QUANTUM *d;
    QUANTUM *s;

    d = dst;
    s = src;

    switch (op) {
    case COMPOSITE_CLEAR:
        // Make white (rgba makes black -- both use 0)
        if ( opacity == OPACITY_OPAQUE ) {
            memset(d, 0, MAX_CHANNEL_CMYKA);
        }
        break;
    case COMPOSITE_COPY:
        if ( opacity == OPACITY_OPAQUE ) {
            memcpy(d, s, MAX_CHANNEL_CMYKA);
        }
        break;
    case COMPOSITE_OVER:
        // XXX: This is _not_ the correct algorithm.
        if (s[PIXEL_CMYK_ALPHA] == OPACITY_TRANSPARENT)
            break;

        if ( opacity == OPACITY_OPAQUE ) {
            if (s[PIXEL_CMYK_ALPHA] == OPACITY_TRANSPARENT ||
                (d[PIXEL_CMYK_ALPHA] == OPACITY_OPAQUE && s[PIXEL_CMYK_ALPHA] == OPACITY_OPAQUE))
            {
                memcpy(d, s, MAX_CHANNEL_CMYKA * sizeof(QUANTUM));
                break;
            }
        }
        alpha = (s[PIXEL_CMYK_ALPHA] * opacity) / QUANTUM_MAX;
        invAlpha = QUANTUM_MAX - alpha;

        d[PIXEL_CYAN] = (d[PIXEL_CYAN] * invAlpha + s[PIXEL_CYAN] * alpha) / QUANTUM_MAX;
        d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA] * invAlpha + s[PIXEL_MAGENTA] * alpha) / QUANTUM_MAX;
        d[PIXEL_YELLOW] = (d[PIXEL_YELLOW] * invAlpha + s[PIXEL_YELLOW] * alpha) / QUANTUM_MAX;
        d[PIXEL_BLACK] = (d[PIXEL_BLACK] * invAlpha + s[PIXEL_BLACK] * alpha) / QUANTUM_MAX;
        alpha = (d[PIXEL_CMYK_ALPHA] * (QUANTUM_MAX - s[PIXEL_CMYK_ALPHA]) + s[PIXEL_CMYK_ALPHA]) / QUANTUM_MAX;
        d[PIXEL_CMYK_ALPHA] = (d[PIXEL_CMYK_ALPHA] * (QUANTUM_MAX - alpha) + s[PIXEL_CMYK_ALPHA]) / QUANTUM_MAX;

        break;
    case COMPOSITE_IN:
    case COMPOSITE_ATOP:
    case COMPOSITE_XOR:
    case COMPOSITE_PLUS:
    case COMPOSITE_MINUS:
    case COMPOSITE_ADD:
    case COMPOSITE_SUBTRACT:
    case COMPOSITE_DIFF:
    case COMPOSITE_MULT:
    case COMPOSITE_BUMPMAP:
    case COMPOSITE_COPY_RED:
    case COMPOSITE_COPY_GREEN:
    case COMPOSITE_COPY_BLUE:
    case COMPOSITE_COPY_OPACITY:
    case COMPOSITE_DISSOLVE:
    case COMPOSITE_DISPLACE:
    case COMPOSITE_MODULATE:
    case COMPOSITE_THRESHOLD:
    default:
        kdDebug() << "Not Implemented.\n";
        return;
    }
}

void KisStrategyColorSpaceCMYK::render(KisImageSP projection,
                                       QPainter& painter,
                                       Q_INT32 x,
                                       Q_INT32 y,
                                       Q_INT32 width,
                                       Q_INT32 height)
{
    // XXX I still don't understand upscale/downscale, so very likely
    // have introduced related bugs in this bit.
    if (projection) {
        KisTileMgrSP tm = projection -> tiles();
        KisPixelDataSP pd = new KisPixelData;
        QImage img;

        pd -> mgr = 0;
        pd -> tile = 0;
        pd -> mode = TILEMODE_READ;
        pd -> x1 = x;
        pd -> x2 = x + width - 1;
        pd -> y1 = y;
        pd -> y2 = y + height - 1;
        pd -> width = pd -> x2 - pd -> x1 + 1;
        pd -> height = pd -> y2 - pd -> y1 + 1;
        pd -> depth = projection -> depth();
        pd -> stride = pd -> depth * pd -> width;
        pd -> owner = false;
        pd -> data = m_buf;
        tm -> readPixelData(pd);
#if 0
        kdDebug() << "ARG: x " << x
                  << ", y " << y
                  << ", w " << width
                  << ", h " << height
                  << endl;
        kdDebug() << "PD: x1 " << pd->x1
                  << ", y1 " << pd->y1
                  << ", x2 " << pd->x2
                  << ", y2  " << pd->y2
                  << ", w " << pd->width
                  << ", h " << pd->height
                  << ", depth " << pd->depth
                  << endl;
#endif
        img = QImage(pd->width,  pd->height, 32, 0, QImage::LittleEndian);
        Q_INT32 i = 0;

        uchar *j = img.bits();
        QString s;
        while ( i < pd ->stride * pd -> height ) {

            RGB r;
            // Check in LUT whether k already exists; if so, grab it, else
            CMYK c;
            c.c = *( pd->data + i + PIXEL_CYAN );
            c.m = *( pd->data + i + PIXEL_MAGENTA );
            c.y = *( pd->data + i + PIXEL_YELLOW );
            c.k = *( pd->data + i + PIXEL_BLACK );

#if 0
            if ( i == 0 ) {
            kdDebug() << "cmyk: "
                      <<  s.sprintf("%02x %02x %02x %02x", c.c, c.m, c.y, c.k) << endl;
            kdDebug() << "alpha " << *( pd->data + i + PIXEL_CMYK_ALPHA ) << endl;
            }
#endif

            if ( m_rgbLUT.contains ( c ) ) {
                r =  m_rgbLUT[c];
            }
            else {
                // Accessing the rgba of KoColor automatically converts
                // from cmyk to rgb and caches the result.
                KoColor k = KoColor(c.c,
                                    c.m,
                                    c.y,
                                    c.k );
                // Store as little as possible
                r.r =  k.R();
                r.g =  k.G();
                r.b =  k.B();
#if 0
                if ( i == 0 ) {
                    kdDebug() << "rgb: "
                              << s.sprintf("#%02x %02x %02x", r.r, r.g, r.b ) << endl;
                }
#endif
                m_rgbLUT[c] = r;
            }

            // fix the pixel in QImage.
            *( j + PIXEL_ALPHA ) = *( pd->data + i + PIXEL_CMYK_ALPHA );
            *( j + PIXEL_RED ) = r.r;
            *( j + PIXEL_GREEN ) =  r.g;
            *( j + PIXEL_BLUE ) =  r.b;

             i += MAX_CHANNEL_CMYKA;
             j += 4; // Because we're hard-coded 32 bits deep, 4 bytes

        }
        m_pixio.putImage(&m_pixmap, 0, 0, &img);
        painter.drawPixmap(x, y, m_pixmap, 0, 0, width, height);
    }
}
