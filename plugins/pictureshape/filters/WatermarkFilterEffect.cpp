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

#include "WatermarkFilterEffect.h"
#include <KoFilterEffectRenderContext.h>
#include <kcolorutils.h>
#include <QColor>

WatermarkFilterEffect::WatermarkFilterEffect()
: KoFilterEffect(WatermarkFilterEffectId,"Watermark Effect")
{
}

WatermarkFilterEffect::~WatermarkFilterEffect()
{
}

void WatermarkFilterEffect::save(KoXmlWriter& /*writer*/)
{
}

bool WatermarkFilterEffect::load(const KoXmlElement& /*element*/, const KoFilterEffectLoadingContext& /*context*/)
{
    return true;
}

QImage WatermarkFilterEffect::processImage(const QImage& image, const KoFilterEffectRenderContext& context) const
{
    QImage result = image.convertToFormat(QImage::Format_ARGB32);
    QRgb*  pixel = reinterpret_cast<QRgb*>( result.bits() );
    const int right = context.filterRegion().right();
    const int bottom = context.filterRegion().bottom();
    const int width = result.width();
    
    for(int row = context.filterRegion().top(); row < bottom; ++row) {
        for(int col = context.filterRegion().left(); col < right; ++col) {
//             const QColor currentPixel = pixel[row * width + col];
//             const QColor currentPixelLighter = KColorUtils::lighten(currentPixel, 0.75);
//             pixel[row * width + col] = currentPixelLighter.rgb();

            quint32 color = pixel[row * width + col];
            quint32 rgb   = 0x00FFFFFF & color;   // get rgb value without alpha
            quint32 alpha = (color >> 25) << 24;  // get alpha value only and divide it by two
            pixel[row * width + col] = alpha|rgb; // set rgb and alpha values to the pixel
        }
    }
    return result;
}
