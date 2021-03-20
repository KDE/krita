/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005-2006 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2004, 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
