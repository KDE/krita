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

#include "kis_round_corners_filter.h"

#include <stdlib.h>
#include <vector>
#include <math.h>

#include <QPoint>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kpluginfactory.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_debug.h>
#include <kis_doc2.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <widgets/kis_multi_integer_filter_widget.h>
#include <kis_selection.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_types.h>
#include <kis_iterator_ng.h>

KisRoundCornersFilter::KisRoundCornersFilter() : KisFilter(id(), KisFilter::categoryMap(), i18n("&Round Corners..."))
{
    setSupportsPainting(false);

}

void KisRoundCornersFilter::process(KisPaintDeviceSP device,
                                    const QRect& applyRect,
                                    const KisFilterConfiguration* config,
                                    KoUpdater* progressUpdater
                                   ) const
{
    Q_UNUSED(config);
    Q_ASSERT(!device.isNull());

    if (!device || !config) {
        warnKrita << "Invalid parameters for round corner filter";
        dbgPlugins << device << " " << config;
        return;
    }

    //read the filter configuration values from the KisFilterConfiguration object
    qint32 radius = qMax(1, config->getInt("radius" , 30));

    if (progressUpdater) {
        progressUpdater->setRange(0, applyRect.height());
    }

    qint32 width = applyRect.width();

    KisHLineIteratorSP dstIt = device->createHLineIteratorNG(applyRect.x(), applyRect.y(), width);

    const KoColorSpace* cs = device->colorSpace();

    QRect bounds = device->defaultBounds()->bounds();
    for (qint32 y = applyRect.y(); y < applyRect.y() + applyRect.height(); y++) {
        qint32 x = applyRect.x();
        do {
            if (x <= radius && y <= radius) {
                double dx = radius - x;
                double dy = radius - y;
                double dradius = static_cast<double>(radius);
                if (dx >= sqrt(dradius*dradius - dy*dy)) {
                    cs->setOpacity(dstIt->rawData(), OPACITY_TRANSPARENT_U8, 1);
                }
            } else if (x >= bounds.width() - radius && y <= radius) {
                double dx = x + radius - bounds.width();
                double dy = radius - y;
                double dradius = static_cast<double>(radius);
                if (dx >= sqrt(dradius*dradius - dy*dy)) {
                    cs->setOpacity(dstIt->rawData(), OPACITY_TRANSPARENT_U8, 1);
                }
            } else if (x <= radius && y >= bounds.height() - radius) {
                double dx = radius - x;
                double dy = y + radius - bounds.height();
                double dradius = static_cast<double>(radius);
                if (dx >= sqrt(dradius*dradius - dy*dy)) {
                    cs->setOpacity(dstIt->rawData(), OPACITY_TRANSPARENT_U8, 1);
                }
            } else if (x >= bounds.width()  - radius && y >= bounds.height() - radius) {

                double dx = x + radius - bounds.width() ;
                double dy = y + radius - bounds.height();
                double dradius = static_cast<double>(radius);
                if (dx >= sqrt(dradius*dradius - dy*dy)) {
                    cs->setOpacity(dstIt->rawData(), OPACITY_TRANSPARENT_U8, 1);
                }
            }
            ++x;
        } while(dstIt->nextPixel());
        dstIt->nextRow();
        if (progressUpdater) progressUpdater->setValue(y);
    }
}

KisConfigWidget * KisRoundCornersFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(2, 100, 30, i18n("Radius"), "radius"));
    return new KisMultiIntegerFilterWidget(id().id(), parent, id().id(), param);

}

KisFilterConfiguration* KisRoundCornersFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("roundcorners", 1);
    config->setProperty("radius", 30);
    return config;
}
