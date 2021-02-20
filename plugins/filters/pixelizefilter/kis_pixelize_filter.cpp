/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from Gimp, SPDX-FileCopyrightText: 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 * original pixelize.c for GIMP 0.54 by Tracy Scott
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_pixelize_filter.h"


#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>
#include <QVector>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoUpdater.h>

#include <kis_debug.h>
#include <KisDocument.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "widgets/kis_multi_integer_filter_widget.h"
#include <KoMixColorsOp.h>
#include <KisSequentialIteratorProgress.h>
#include "kis_algebra_2d.h"
#include "kis_lod_transform.h"


KisPixelizeFilter::KisPixelizeFilter() : KisFilter(id(), FiltersCategoryArtisticId, i18n("&Pixelize..."))
{
    setSupportsPainting(true);
    setSupportsThreading(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

void KisPixelizeFilter::processImpl(KisPaintDeviceSP device,
                                    const QRect& applyRect,
                                    const KisFilterConfigurationSP config,
                                    KoUpdater* progressUpdater
                                    ) const
{
    Q_ASSERT(device);

    KisLodTransformScalar t(device);
    const int pixelWidth = qCeil(t.scale(config ? qMax(1, config->getInt("pixelWidth", 10)) : 10));
    const int pixelHeight = qCeil(t.scale(config ? qMax(1, config->getInt("pixelHeight", 10)) : 10));

    const qint32 pixelSize = device->pixelSize();

    const QRect deviceBounds = device->defaultBounds()->bounds();

    const int bufferSize = pixelSize * pixelWidth * pixelHeight;
    QScopedArrayPointer<quint8> buffer(new quint8[bufferSize]);

    KoColor pixelColor(Qt::black, device->colorSpace());
    KoMixColorsOp *mixOp = device->colorSpace()->mixColorsOp();

    using namespace KisAlgebra2D;
    const qint32 firstCol = divideFloor(applyRect.x(), pixelWidth);
    const qint32 firstRow = divideFloor(applyRect.y(), pixelHeight);

    const qint32 lastCol = divideFloor(applyRect.x() + applyRect.width() - 1, pixelWidth);
    const qint32 lastRow = divideFloor(applyRect.y() + applyRect.height() - 1, pixelHeight);

    progressUpdater->setRange(firstRow, lastRow);

    for(qint32 i = firstRow; i <= lastRow; i++) {
        for(qint32 j = firstCol; j <= lastCol; j++) {
            const QRect maxPatchRect(j * pixelWidth, i * pixelHeight,
                                     pixelWidth, pixelHeight);
            const QRect pixelRect = maxPatchRect & deviceBounds;
            const int numColors = pixelRect.width() * pixelRect.height();


            //read
            KisSequentialConstIterator srcIt(device, pixelRect);

            memset(buffer.data(), 0, bufferSize);
            quint8 *bufferPtr = buffer.data();

            while (srcIt.nextPixel()) {
                memcpy(bufferPtr, srcIt.oldRawData(), pixelSize);
                bufferPtr += pixelSize;
            }

            // mix all the colors
            mixOp->mixColors(buffer.data(), numColors, pixelColor.data());

            // write only colors in applyRect
            const QRect writeRect = pixelRect & applyRect;

            KisSequentialIterator dstIt(device, writeRect);
            while (dstIt.nextPixel()) {
                memcpy(dstIt.rawData(), pixelColor.data(), pixelSize);
            }
        }
        progressUpdater->setValue(i);
    }
}

QRect KisPixelizeFilter::neededRect(const QRect &rect, const KisFilterConfigurationSP config, int lod) const
{
    KisLodTransformScalar t(lod);

    const int pixelWidth = qCeil(t.scale(config ? qMax(1, config->getInt("pixelWidth", 10)) : 10));
    const int pixelHeight = qCeil(t.scale(config ? qMax(1, config->getInt("pixelHeight", 10)) : 10));

    // TODO: make more precise calculation of the rect, including the alignment
    return rect.adjusted(-2*pixelWidth, -2*pixelHeight, 2*pixelWidth, 2*pixelHeight);
}

QRect KisPixelizeFilter::changedRect(const QRect &rect, const KisFilterConfigurationSP config, int lod) const
{
    return neededRect(rect, config, lod);
}

KisConfigWidget * KisPixelizeFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(2, 512, 10, i18n("Pixel width"), "pixelWidth"));
    param.push_back(KisIntegerWidgetParam(2, 512, 10, i18n("Pixel height"), "pixelHeight"));
    return new KisMultiIntegerFilterWidget(id().id(),  parent,  id().id(),  param);
}

KisFilterConfigurationSP KisPixelizeFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("pixelWidth", 10);
    config->setProperty("pixelHeight", 10);
    return config;
}

