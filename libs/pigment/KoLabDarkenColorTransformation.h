/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005-2006 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_LAB_DARKEN_COLOR_TRANSFORMATION_H_
#define _KO_LAB_DARKEN_COLOR_TRANSFORMATION_H_

#if !defined _MSC_VER
#pragma GCC diagnostic ignored "-Wcast-align"
#endif

#include "KoColorTransformation.h"

template<typename _lab_channels_type_>
struct KoLabDarkenColorTransformation : public KoColorTransformation {
    KoLabDarkenColorTransformation(qint32 shade, bool compensate, qreal compensation, const KoColorSpace *colorspace)
        : m_colorSpace(colorspace)
        , m_shade(shade)
        , m_compensate(compensate)
        , m_compensation(compensation)
    {
    }

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        *((quint32 *)dst) = *((const quint32 *)src);

        QColor c;

        for (unsigned int i = 0; i < nPixels*m_colorSpace->pixelSize(); i+=m_colorSpace->pixelSize()) {
            if (m_compensate) {
                m_colorSpace->toQColor(src+i,&c);
                c.setRed((c.red()*m_shade)/(m_compensation*255));
                c.setGreen((c.green()*m_shade)/(m_compensation*255));
                c.setBlue((c.blue()*m_shade)/(m_compensation*255));
                m_colorSpace->fromQColor(c,dst+i);
            } else {
                m_colorSpace->toQColor(src+i,&c);
                c.setRed((c.red()*m_shade)/255);
                c.setGreen((c.green()*m_shade)/255);
                c.setBlue((c.blue()*m_shade)/255);
                m_colorSpace->fromQColor(c,dst+i);
            }
        }
    }
    const KoColorSpace* m_colorSpace {0};
    const KoColorConversionTransformation* m_defaultToLab {0};
    const KoColorConversionTransformation* m_defaultFromLab {0};
    qint32 m_shade {0};
    bool m_compensate {false};
    qreal m_compensation {0.0};
};

#endif
