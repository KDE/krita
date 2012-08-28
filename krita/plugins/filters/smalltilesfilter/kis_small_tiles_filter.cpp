/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from Gimp, Copyright (C) 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 * original pixelize.c for GIMP 0.54 by Tracy Scott
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_small_tiles_filter.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>
#include <QVector>

#include <klocale.h>
#include <kpluginfactory.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_doc2.h>
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
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KoCompositeOp.h>

#include "widgets/kis_multi_integer_filter_widget.h"

KisSmallTilesFilter::KisSmallTilesFilter() : KisFilter(id(), KisFilter::categoryMap(), i18n("&Small Tiles..."))
{
    setSupportsPainting(true);
    setSupportsIncrementalPainting(false);
    setSupportsThreading(false);
}

void KisSmallTilesFilter::process(KisPaintDeviceSP device,
                                  const QRect& /*applyRect*/,
                                  const KisFilterConfiguration* config,
                                  KoUpdater* progressUpdater
                                 ) const
{
    Q_ASSERT(!device.isNull());

    //read the filter configuration values from the KisFilterConfiguration object
    quint32 numberOfTiles = config->getInt("numberOfTiles", 2);

    QRect srcRect = device->exactBounds();

    int w = static_cast<int>(srcRect.width() / numberOfTiles);
    int h = static_cast<int>(srcRect.height() / numberOfTiles);

    KisPaintDeviceSP tile = device->createThumbnailDevice(srcRect.width() / numberOfTiles, srcRect.height() / numberOfTiles);
    if (tile.isNull()) return;

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

KisConfigWidget * KisSmallTilesFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(2, 5, 1, i18n("Number of tiles"), "numberOfTiles"));
    return new KisMultiIntegerFilterWidget(id().id(),  parent,  id().id(),  param);

}

KisFilterConfiguration* KisSmallTilesFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("smalltiles", 1);
    config->setProperty("numberOfTiles", 2);
    return config;
}


