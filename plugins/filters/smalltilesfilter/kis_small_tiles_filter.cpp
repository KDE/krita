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

#include "kis_small_tiles_filter.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>
#include <QVector>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoUpdater.h>

#include <KisDocument.h>
#include <kis_debug.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_filter_strategy.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KoCompositeOpRegistry.h>

#include "widgets/kis_multi_integer_filter_widget.h"

KisSmallTilesFilter::KisSmallTilesFilter() : KisFilter(id(), FiltersCategoryMapId, i18n("&Small Tiles..."))
{
    setSupportsPainting(true);
    setSupportsThreading(false);
    setSupportsAdjustmentLayers(false);
}

void KisSmallTilesFilter::processImpl(KisPaintDeviceSP device,
                                      const QRect& applyRect,
                                      const KisFilterConfigurationSP config,
                                      KoUpdater* progressUpdater
                                      ) const
{
    Q_ASSERT(!device.isNull());

    //read the filter configuration values from the KisFilterConfiguration object
    const quint32 numberOfTiles = config->getInt("numberOfTiles", 2);

    const QRect srcRect = applyRect;

    const int w = static_cast<int>(srcRect.width() / numberOfTiles);
    const int h = static_cast<int>(srcRect.height() / numberOfTiles);

    KisPaintDeviceSP tile = device->createThumbnailDevice(w, h);
    if (tile.isNull()) return;

    device->clear(applyRect);

    KisPainter gc(device);
    gc.setCompositeOp(COMPOSITE_COPY);

    if (progressUpdater) {
        progressUpdater->setRange(0, numberOfTiles);
    }

    for (uint y = 0; y < numberOfTiles; ++y) {
        for (uint x = 0; x < numberOfTiles; ++x) {
            gc.bitBlt(w * x, h * y, tile, 0, 0, w, h);
        }
        if (progressUpdater) progressUpdater->setValue(y);
    }
    gc.end();
}

KisConfigWidget * KisSmallTilesFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(2, 5, 1, i18n("Number of tiles"), "numberOfTiles"));
    return new KisMultiIntegerFilterWidget(id().id(),  parent,  id().id(),  param);

}

KisFilterConfigurationSP KisSmallTilesFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("numberOfTiles", 2);
    return config;
}


