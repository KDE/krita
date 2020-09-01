#include "KisFilterFastColorOverlay.h"

#include "kis_paint_device.h"
#include "kis_random_accessor_ng.h"
#include "KoCompositeOp.h"
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include <KoColor.h>

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>

#include "KisWdgFilterFastColorOverlay.h"

KisFilterFastColorOverlay::KisFilterFastColorOverlay()
    : KisFilter(id(), FiltersCategoryColorId, i18n("Fast Color &Overlay..."))
{

}

void KisFilterFastColorOverlay::processImpl(KisPaintDeviceSP device, const QRect &rect, const KisFilterConfigurationSP config, KoUpdater *progressUpdater) const
{
    Q_UNUSED(progressUpdater);

    const KoColorSpace *colorSpace = device->colorSpace();

    KoColor overlayColor = config->getColor("color", KoColor(QColor(185, 221, 255), KoColorSpaceRegistry::instance()->rgb8()));
    overlayColor.convertTo(colorSpace);

    KoCompositeOp::ParameterInfo paramInfo;
    paramInfo.opacity = config->getPropertyLazy("opacity", 75) / 100.0f;
    paramInfo.flow = 1.0;
    paramInfo.channelFlags = colorSpace->channelFlags(true, false);

    const KoCompositeOp *compositeOp =
        colorSpace->compositeOp(config->getPropertyLazy("compositeop", COMPOSITE_OVER));

    KisRandomAccessorSP dstIt = device->createRandomAccessorNG();

    qint32 dstY_ = rect.y();
    qint32 rowsRemaining = rect.height();

    while (rowsRemaining > 0) {

        qint32 dstX_ = rect.x();
        qint32 columnsRemaining = rect.width();
        qint32 numContiguousDstRows = dstIt->numContiguousRows(dstY_);

        qint32 rows = qMin(numContiguousDstRows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousDstColumns = dstIt->numContiguousColumns(dstX_);
            qint32 columns = qMin(numContiguousDstColumns, columnsRemaining);

            qint32 dstRowStride = dstIt->rowStride(dstX_, dstY_);
            dstIt->moveTo(dstX_, dstY_);

            paramInfo.dstRowStart   = dstIt->rawData();
            paramInfo.dstRowStride  = dstRowStride;
            paramInfo.srcRowStart   = overlayColor.data();
            paramInfo.srcRowStride  = 0;
            paramInfo.maskRowStart  = 0;
            paramInfo.maskRowStride = 0;
            paramInfo.rows          = rows;
            paramInfo.cols          = columns;
            colorSpace->bitBlt(overlayColor.colorSpace(), paramInfo, compositeOp, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());

            dstX_ += columns;
            columnsRemaining -= columns;
        }

        dstY_ += rows;
        rowsRemaining -= rows;
    }

}

KisConfigWidget *KisFilterFastColorOverlay::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const
{
    Q_UNUSED(useForMasks);
    return new KisWdgFilterFastColorOverlay(parent);
}

KisFilterConfigurationSP KisFilterFastColorOverlay::defaultConfiguration() const
{
    KisFilterConfigurationSP config = factoryConfiguration();
    config->setProperty("color", QColor(185, 221, 255));
    config->setProperty("opacity", 75);
    config->setProperty("compositeop", COMPOSITE_OVER);
    return config;
}
