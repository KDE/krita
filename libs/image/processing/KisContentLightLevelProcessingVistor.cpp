/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisContentLightLevelProcessingVistor.h"
#include <kis_hdr_metadata.h>
#include "kis_group_layer.h"
#include "kis_iterator_ng.h"
#include "kis_raster_keyframe_channel.h"


struct KisContentLightLevelProcessingVistor::Private {
    QList<KisContentLightLevelInformation> clli;
};

KisContentLightLevelProcessingVistor::KisContentLightLevelProcessingVistor()
    : KisDoNothingProcessingVisitor(), d(new Private)
{
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
    /**
     * @brief xyzCS
     * We need the luminance, so we could either do complex luminance calculations
     * for each and every color space... Or we could convert to linear XYZ and only
     * test the Y value.
     */
    const KoColorSpace *xyzCS = KoColorSpaceRegistry::instance()->colorSpace(XYZAColorModelID.id(), Float32BitsColorDepthID.id());
    dev->convertTo(xyzCS, KoColorConversionTransformation::IntentRelativeColorimetric);

    QRectF imageBounds = dev->extent();
    KisHLineConstIteratorSP it = dev->createHLineConstIteratorNG(imageBounds.x(), imageBounds.y(), imageBounds.width());

    KisContentLightLevelInformation clli;
    double average = 0.0;
    int divider = 0;

    qDebug() << "preparing frame" << imageBounds;
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

    clli.maxFrameAverageLightLevel = average / divider;
    qDebug() << "handled frame" << clli.maxContentLightLevel << clli.maxFrameAverageLightLevel;
    return clli;
}
