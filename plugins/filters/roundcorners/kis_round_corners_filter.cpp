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

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoUpdater.h>

#include <kis_debug.h>
#include <KisDocument.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <widgets/kis_multi_integer_filter_widget.h>
#include <kis_selection.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_types.h>
#include <KisSequentialIteratorProgress.h>
#include <kis_algebra_2d.h>
#include <KoProgressUpdater.h>


KisRoundCornersFilter::KisRoundCornersFilter() : KisFilter(id(), FiltersCategoryMapId, i18n("&Round Corners..."))
{
    setSupportsPainting(false);

}

void fadeOneCorner(KisPaintDeviceSP device,
                   const QPoint &basePoint,
                   const QRect &processRect,
                   const qreal thresholdSq,
                   KoUpdater* progressUpdater)
{
    const KoColorSpace *cs = device->colorSpace();
    KisSequentialIteratorProgress dstIt(device, processRect, progressUpdater);

    while (dstIt.nextPixel()) {
        const QPointF point(dstIt.x(), dstIt.y());

        const qreal distanceSq = kisSquareDistance(point, basePoint);
        if (distanceSq >= thresholdSq) {
            cs->setOpacity(dstIt.rawData(), OPACITY_TRANSPARENT_U8, 1);
        }
    }
}


void KisRoundCornersFilter::processImpl(KisPaintDeviceSP device,
                                        const QRect& applyRect,
                                        const KisFilterConfigurationSP config,
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

    const QRect bounds = device->defaultBounds()->bounds();

    const qint32 radius = qMin(KisAlgebra2D::minDimension(bounds) / 2, qMax(1, config->getInt("radius" , 30)));
    const qreal radiusSq = pow2(radius);

    struct CornerJob {
        QRect rc;
        QPoint pt;
        KoUpdater *progressUpdater;
    };

    QVector<CornerJob> jobs;

    KoProgressUpdater compositeUpdater(progressUpdater, KoProgressUpdater::Unthreaded);

    {
        QRect rc(bounds.x(), bounds.y(), radius, radius);
        QPoint pt(rc.bottomRight());
        jobs << CornerJob({rc, pt, compositeUpdater.startSubtask()});
    }

    {
        QRect rc(bounds.x() + bounds.width() - radius, bounds.y(), radius, radius);
        QPoint pt(rc.bottomLeft());
        jobs << CornerJob({rc, pt, compositeUpdater.startSubtask()});
    }

    {
        QRect rc(bounds.x(), bounds.y() + bounds.height() - radius, radius, radius);
        QPoint pt(rc.topRight());
        jobs << CornerJob({rc, pt, compositeUpdater.startSubtask()});
    }

    {
        QRect rc(bounds.x() + bounds.width() - radius, bounds.y() + bounds.height() - radius, radius, radius);
        QPoint pt(rc.topLeft());
        jobs << CornerJob({rc, pt, compositeUpdater.startSubtask()});
    }

    Q_FOREACH (const CornerJob &job, jobs) {
        const QRect processRect = job.rc & applyRect;
        if (!processRect.isEmpty()) {
            fadeOneCorner(device, job.pt, processRect, radiusSq, job.progressUpdater);
        }
    }
}

KisConfigWidget * KisRoundCornersFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(2, 100, 30, i18n("Radius"), "radius"));
    return new KisMultiIntegerFilterWidget(id().id(), parent, id().id(), param);

}

KisFilterConfigurationSP KisRoundCornersFilter::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("roundcorners", 1);
    config->setProperty("radius", 30);
    return config;
}
