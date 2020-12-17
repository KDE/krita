/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
