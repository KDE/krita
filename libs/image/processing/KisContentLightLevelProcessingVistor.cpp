/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisContentLightLevelProcessingVistor.h"
#include <kis_hdr_metadata.h>
#include <KoColorProfile.h>
#include <KoColorProfileQuery.h>
#include "kis_group_layer.h"
#include "kis_sequential_iterator.h"


struct KisContentLightLevelProcessingVistor::Private {
    KisRelativeContentLightLevelInformation::CalculationType type;
    QRect cropRect;
    QList<KisRelativeContentLightLevelInformation> clli;
};

KisContentLightLevelProcessingVistor::KisContentLightLevelProcessingVistor(KisRelativeContentLightLevelInformation::CalculationType type, const QRect &cropRect)
    : KisDoNothingProcessingVisitor(), d(new Private)
{
    d->type = type;
    d->cropRect = cropRect;
}

KisContentLightLevelProcessingVistor::~KisContentLightLevelProcessingVistor()
{

}

void KisContentLightLevelProcessingVistor::visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(undoAdapter)
    if (layer->parent()) return;

    KisPaintDeviceSP projection = layer->projection();
    KisPaintDeviceSP dev = new KisPaintDevice(layer->colorSpace());
    dev->makeCloneFrom(projection, projection->extent());
    d->clli.append(calculateForDev(dev));
}

KisRelativeContentLightLevelInformation KisContentLightLevelProcessingVistor::contentLightLevelInformation() const
{
    KisRelativeContentLightLevelInformation clli;
    double total = 0.0;
    Q_FOREACH(const KisRelativeContentLightLevelInformation c, d->clli) {
        clli.maxContentLightLevel = qMax(clli.maxContentLightLevel, c.maxContentLightLevel);
        total += c.maxFrameAverageLightLevel;
    }
    clli.maxFrameAverageLightLevel = d->clli.isEmpty()? 0.0: total / d->clli.size();
    clli.type = d->type;
    return clli;
}

KisRelativeContentLightLevelInformation KisContentLightLevelProcessingVistor::calculateForDev(KisPaintDeviceSP dev)
{
    KisRelativeContentLightLevelInformation clli;
    double average = 0.0;
    int divider = 0;
    const QRect imageBounds = d->cropRect.isValid()? d->cropRect: dev->exactBounds();

    bool canConvertLinear = dev->colorSpace()->colorModelId() == RGBAColorModelID && dev->colorSpace()->profile()->getColorPrimaries() != PRIMARIES_UNSPECIFIED;

    if ((d->type == KisRelativeContentLightLevelInformation::RGBComponent && canConvertLinear)
        || d->type == KisRelativeContentLightLevelInformation::Rec2020Component){
        if (d->type == KisRelativeContentLightLevelInformation::Rec2020Component) {
            const KoColorSpace *rec2020CS = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                         Float32BitsColorDepthID.id(),
                                                                                         KoColorSpaceRegistry::instance()->p2020G10Profile());
            dev->convertTo(rec2020CS, KoColorConversionTransformation::IntentRelativeColorimetric);
        } else {
            const KoColorProfile *linP = KoColorSpaceRegistry::instance()->profileFor(KoColorProfileQuery(dev->colorSpace()->profile()->getColorPrimaries(), TRC_LINEAR), true);
            const KoColorSpace *linear = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                         Float32BitsColorDepthID.id(),
                                                                                         linP);
            dev->convertTo(linear, KoColorConversionTransformation::IntentRelativeColorimetric);
        }
        KisSequentialConstIterator srcIt(dev, imageBounds);

        QVector<float> channels(dev->colorSpace()->channelCount());
        while(srcIt.nextPixel()) {
            const quint8* pixel = srcIt.rawDataConst();
            dev->colorSpace()->normalisedChannelsValue(pixel, channels);

            const double rgbMax = qMax(channels[0], qMax(channels[1], channels[2]));

            average += rgbMax;
            divider ++;
            clli.maxContentLightLevel = qMax(clli.maxContentLightLevel, rgbMax);
        }
    } else {
        /**
         * @brief xyzCS
         * We need the luminance, so we could either do complex luminance calculations
         * for each and every color space... Or we could convert to linear XYZ and only
         * test the Y value.
         */
        const KoColorSpace *xyzCS = KoColorSpaceRegistry::instance()->colorSpace(XYZAColorModelID.id(), Float32BitsColorDepthID.id());
        dev->convertTo(xyzCS, KoColorConversionTransformation::IntentRelativeColorimetric);

        KisSequentialConstIterator srcIt(dev, imageBounds);

        while (srcIt.nextPixel()) {
            const KoXyzF32Traits::Pixel* pixel = reinterpret_cast<const KoXyzF32Traits::Pixel*> (srcIt.rawDataConst());
            clli.maxContentLightLevel = qMax(clli.maxContentLightLevel, double(pixel->y));
            average += pixel->y;
            divider ++;
        }
    }

    clli.maxFrameAverageLightLevel = divider > 0? average / divider: 0;
    return clli;
}
