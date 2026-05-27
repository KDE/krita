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
#include "kis_iterator_ng.h"
#include "kis_raster_keyframe_channel.h"


struct KisContentLightLevelProcessingVistor::Private {
    KisContentLightLevelInformation::CalculationType type;
    QList<KisContentLightLevelInformation> clli;
};

KisContentLightLevelProcessingVistor::KisContentLightLevelProcessingVistor(KisContentLightLevelInformation::CalculationType type)
    : KisDoNothingProcessingVisitor(), d(new Private)
{
    d->type = type;
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

KisContentLightLevelInformation KisContentLightLevelProcessingVistor::contentLightLevelInformation() const
{
    KisContentLightLevelInformation clli;
    double total = 0.0;
    for(int i = 0; i < d->clli.size(); i++) {
        KisContentLightLevelInformation c = d->clli.at(i);
        clli.maxContentLightLevel = qMax(clli.maxContentLightLevel, c.maxContentLightLevel);
        total += c.maxFrameAverageLightLevel;
    }
    clli.maxFrameAverageLightLevel = d->clli.isEmpty()? 0.0: total / d->clli.size();
    return clli;
}

KisContentLightLevelInformation KisContentLightLevelProcessingVistor::calculateForDev(KisPaintDeviceSP dev)
{
    KisContentLightLevelInformation clli;
    double average = 0.0;
    int divider = 0;
    const QRectF imageBounds = dev->extent();
    qDebug() << "preparing frame" << imageBounds;

    bool canConvertLinear = dev->colorSpace()->colorModelId() == RGBAColorModelID && dev->colorSpace()->profile()->getColorPrimaries() != PRIMARIES_UNSPECIFIED;

    if ((d->type == KisContentLightLevelInformation::RGBComponent && canConvertLinear)
        || d->type == KisContentLightLevelInformation::Rec2020Component){
        if (d->type == KisContentLightLevelInformation::Rec2020Component) {
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
        KisHLineConstIteratorSP it = dev->createHLineConstIteratorNG(imageBounds.x(), imageBounds.y(), imageBounds.width());

        QVector<float> channels(dev->colorSpace()->channelCount());
        for (int y = 0; y < imageBounds.height(); y++) {
            for (int x = 0; x < imageBounds.width(); x++) {
                const quint8* pixel = it->rawDataConst();
                dev->colorSpace()->normalisedChannelsValue(pixel, channels);

                const double rgbMax = qMax(channels[0], qMax(channels[1], channels[2]));

                average += rgbMax;
                divider ++;
                clli.maxContentLightLevel = qMax(clli.maxContentLightLevel, rgbMax);
                it->nextPixel();
            }
            it->nextRow();
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

        KisHLineConstIteratorSP it = dev->createHLineConstIteratorNG(imageBounds.x(), imageBounds.y(), imageBounds.width());

        for (int y = 0; y < imageBounds.height(); y++) {
            for (int x = 0; x < imageBounds.width(); x++) {
                const KoXyzF32Traits::Pixel* pixel = reinterpret_cast<const KoXyzF32Traits::Pixel*> (it->rawDataConst());
                clli.maxContentLightLevel = qMax(clli.maxContentLightLevel, double(pixel->y));
                average += pixel->y;
                divider ++;
                it->nextPixel();
            }
            it->nextRow();
        }
    }

    clli.maxFrameAverageLightLevel = average / divider;
    qDebug() << "handled frame" << clli.maxContentLightLevel << clli.maxFrameAverageLightLevel;
    return clli;
}
