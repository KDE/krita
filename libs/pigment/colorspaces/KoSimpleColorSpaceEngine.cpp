/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoSimpleColorSpaceEngine.h"

#include "KoColorModelStandardIds.h"

#include <klocalizedstring.h>

#include "KoColorSpace.h"

#include "DebugPigment.h"

#include <QColor>

// -- KoSimpleColorConversionTransformation --

class KoSimpleColorConversionTransformation : public KoColorConversionTransformation
{
public:
    KoSimpleColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs)
        : KoColorConversionTransformation(srcCs, dstCs,
                                          KoColorConversionTransformation::internalRenderingIntent(),
                                          KoColorConversionTransformation::internalConversionFlags()) {
    }

    ~KoSimpleColorConversionTransformation() override {
    }

    void transform(const quint8 *src, quint8 *dst, qint32 numPixels) const override {
        const KoColorSpace* srcCs = srcColorSpace();
        const KoColorSpace* dstCs = dstColorSpace();

        quint32 srcPixelsize = srcCs->pixelSize();
        quint32 dstPixelsize = dstCs->pixelSize();

        QColor c;
        while (numPixels > 0) {

            srcCs->toQColor(src, &c);
            dstCs->fromQColor(c, dst);

            src += srcPixelsize;
            dst += dstPixelsize;

            --numPixels;
        }
    }

};



KoSimpleColorSpaceEngine::KoSimpleColorSpaceEngine()
    : KoColorSpaceEngine("simple", i18n("Simple Color Conversion Engine"))
{
}

KoSimpleColorSpaceEngine::~KoSimpleColorSpaceEngine()
{
}

KoColorConversionTransformation* KoSimpleColorSpaceEngine::createColorTransformation(const KoColorSpace* srcColorSpace,
                                                                                     const KoColorSpace* dstColorSpace,
                                                                                     KoColorConversionTransformation::Intent renderingIntent,
                                                                                     KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    Q_UNUSED(renderingIntent);
    Q_UNUSED(conversionFlags);
    return new KoSimpleColorConversionTransformation(srcColorSpace, dstColorSpace);
}
