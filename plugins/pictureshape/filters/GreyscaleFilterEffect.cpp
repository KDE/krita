/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
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
    QImage result = image;
    QRgb* pixel = reinterpret_cast<QRgb*>( result.bits() );
    const int right = context.filterRegion().right();
    const int bottom = context.filterRegion().bottom();
    const int width = result.width();
    for( int row = context.filterRegion().top(); row < bottom; ++row ) {
        for( int col = context.filterRegion().left(); col < right; ++col ){
            const QRgb currentPixel = pixel[row * width + col];
            const int red = qRed(currentPixel);
            const int green = qGreen(currentPixel);
            const int blue = qBlue(currentPixel);
            const int grayValue = (red * 11 + green * 16 + blue * 5) / 32;
            pixel[row * width + col] = qRgb(grayValue, grayValue, grayValue);
        }
    }
    return result;
}
