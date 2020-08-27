#include "KisFilterFastColorOverlay.h"

#include "kis_paint_device.h"
#include "kis_random_accessor_ng.h"
#include "KoCompositeOp.h"
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include <KoColor.h>

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>

KisFilterFastColorOverlay::KisFilterFastColorOverlay()
    : KisFilter(id(), FiltersCategoryColorId, i18n("Fast Color &Overlay..."))
{

}

void KisFilterFastColorOverlay::processImpl(KisPaintDeviceSP device, const QRect &rect, const KisFilterConfigurationSP config, KoUpdater *progressUpdater) const
{
    Q_UNUSED(config);
    Q_UNUSED(progressUpdater);

    const KoColorSpace *colorSpace = device->colorSpace();

    KoColor overlayColor(QColor(138, 212, 228), colorSpace);

    KoCompositeOp::ParameterInfo paramInfo;
    paramInfo.opacity = 1.0;
    paramInfo.flow = 1.0;
    paramInfo.channelFlags = colorSpace->channelFlags(true, false);

    const KoCompositeOp *compositeOp = colorSpace->compositeOp(COMPOSITE_OVER);

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
    return 0;
}

KisFilterConfigurationSP KisFilterFastColorOverlay::defaultConfiguration() const
{
    KisFilterConfigurationSP config = factoryConfiguration();
    config->setProperty("overlaycolor", QColor(138, 212, 228));
    config->setProperty("opacity", 100);
    return config;
}
