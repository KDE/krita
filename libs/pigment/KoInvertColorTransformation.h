/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KO_INVERT_COLOR_TRANSFORMATION_H
#define KO_INVERT_COLOR_TRANSFORMATION_H

class KoInvertColorTransformation : public KoColorTransformation
{

public:

    KoInvertColorTransformation(const KoColorSpace* cs) : m_colorSpace(cs), m_psize(cs->pixelSize()) {
    }

    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const {
        quint16 m_rgba[4];
        while (nPixels--) {
            m_colorSpace->toRgbA16(src, reinterpret_cast<quint8 *>(m_rgba), 1);
            m_rgba[0] = KoColorSpaceMathsTraits<quint16>::max - m_rgba[0];
            m_rgba[1] = KoColorSpaceMathsTraits<quint16>::max - m_rgba[1];
            m_rgba[2] = KoColorSpaceMathsTraits<quint16>::max - m_rgba[2];
            m_colorSpace->fromRgbA16(reinterpret_cast<quint8 *>(m_rgba), dst, 1);
            src += m_psize;
            dst += m_psize;
        }

    }

private:

    const KoColorSpace* m_colorSpace;
    quint32 m_psize;
};


#endif
