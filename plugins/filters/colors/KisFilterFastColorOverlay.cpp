/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

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

    KoColor overlayColor = config->getColor("color", KoColor(defaultColor(), KoColorSpaceRegistry::instance()->rgb8()));
    overlayColor.convertTo(colorSpace);

    KoCompositeOp::ParameterInfo paramInfo;
    paramInfo.opacity = config->getPropertyLazy("opacity", defaultOpacity()) / 100.0f;
    paramInfo.flow = 1.0;
    paramInfo.channelFlags = colorSpace->channelFlags(true, false);

    const KoCompositeOp *compositeOp =
        colorSpace->compositeOp(config->getPropertyLazy("compositeop", defaultCompositeOp()));

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
    Q_UNUSED(dev);
    Q_UNUSED(useForMasks);
    return new KisWdgFilterFastColorOverlay(parent);
}

KisFilterConfigurationSP KisFilterFastColorOverlay::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("color", defaultColor());
    config->setProperty("opacity", defaultOpacity());
    config->setProperty("compositeop", defaultCompositeOp());
    return config;
}

QColor KisFilterFastColorOverlay::defaultColor()
{
    return QColor(62, 140, 236);
}

int KisFilterFastColorOverlay::defaultOpacity()
{
    return 100;
}

QString KisFilterFastColorOverlay::defaultCompositeOp()
{
    // Use the normal blending mode (OVER is "normal").
    // This offers the best performance, but will change the color of all opaque pixels,
    // which may be unexpected in some situations (for example, coloring a scanned sketch on a white background).
    return COMPOSITE_OVER;
}
