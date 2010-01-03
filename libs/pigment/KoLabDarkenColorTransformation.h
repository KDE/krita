/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005-2006 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_LAB_DARKEN_TRANSFORMATION_H_
#define _KO_LAB_DARKEN_TRANSFORMATION_H_

template<typename _lab_channels_type_>
struct KoLabDarkenColorTransformation : public KoColorTransformation {
    KoLabDarkenColorTransformation(qint32 shade, bool compensate, qreal compensation) : m_shade(shade), m_compensate(compensate), m_compensation(compensation) {

    }
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const {
        const _lab_channels_type_ * srcNT = reinterpret_cast<const _lab_channels_type_*>(src);
        _lab_channels_type_ * dstNT = reinterpret_cast<_lab_channels_type_*>(dst);
        for (int i = 0; i < nPixels * 4; ++i) {
            if (m_compensate) {
                dstNT[i] = static_cast<quint16>((srcNT[i] * m_shade) / (m_compensation * 255));
            } else {
                dstNT[i] = static_cast<quint16>(srcNT[i] * m_shade  / 255);
            }
        }
    }
    const KoColorSpace* m_colorSpace;
    const KoColorConversionTransformation* m_defaultToLab;
    const KoColorConversionTransformation* m_defaultFromLab;
    qint32 m_shade;
    bool m_compensate;
    qreal m_compensation;
};

#endif
