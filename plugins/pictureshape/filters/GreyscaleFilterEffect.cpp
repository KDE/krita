/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include "GreyscaleFilterEffect.h"
#include <KoFilterEffectRenderContext.h>

#include <QImage>

GreyscaleFilterEffect::GreyscaleFilterEffect()
: KoFilterEffect(GreyscaleFilterEffectId, "Grayscale effect")
{
}

GreyscaleFilterEffect::~GreyscaleFilterEffect()
{
}

void GreyscaleFilterEffect::save(KoXmlWriter& /*writer*/)
{
}

bool GreyscaleFilterEffect::load(const KoXmlElement& /*element*/, const KoFilterEffectLoadingContext& /*context*/)
{
    return true;
}

QImage GreyscaleFilterEffect::processImage(const QImage& image, const KoFilterEffectRenderContext& context) const
{
    QImage result = image.convertToFormat(QImage::Format_ARGB32);

    const int bottom = context.filterRegion().bottom();
    const int left = context.filterRegion().left();
    const int right = context.filterRegion().right();
    const int width = result.width();
    const QRgb *src = (const QRgb*)image.constBits();
    QRgb *dst = (QRgb*)result.bits();

    for (int row = context.filterRegion().top(); row < bottom; ++row) {
        for (int col = left; col < right; ++col) {
            int index = row * width + col;
            const QRgb &s = src[index];
            const int red = qRed(s);
            const int green = qGreen(s);
            const int blue = qBlue(s);
            const int alpha = qAlpha(s);
            const int grayValue = (red * 11 + green * 16 + blue * 5) / 32;
            dst[index] = qRgba(grayValue, grayValue, grayValue, alpha);
        }
    }
    return result;
}
