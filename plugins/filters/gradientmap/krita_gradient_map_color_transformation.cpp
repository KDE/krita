/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
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

#include "krita_gradient_map_color_transformation.h"
#include "KoColor.h"
#include "KoColorSpaceMaths.h"

KritaGradientMapColorTransformation::KritaGradientMapColorTransformation(const KoAbstractGradient * gradient, const KoColorSpace * cs)
    : m_gradient(gradient),
      m_colorSpace(cs),
      m_psize(cs->pixelSize())
{
}

void KritaGradientMapColorTransformation::transform(const quint8 * src, quint8 * dst, qint32 nPixels) const
{
    quint16 srcPixel[4];
    qreal grey;
    KoColor outColor(m_colorSpace);

    while (nPixels--)
    {
        m_colorSpace->toLabA16(src,reinterpret_cast<quint8 *>(srcPixel),1);
        grey = qreal(srcPixel[0]) / KoColorSpaceMathsTraits<quint16>::max;
        m_gradient->colorAt(outColor, grey);
        memcpy(dst, outColor.data(), m_psize);
        src += m_psize;
        dst += m_psize;
    }
}

