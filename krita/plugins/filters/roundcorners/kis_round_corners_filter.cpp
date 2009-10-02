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
#include <kgenericfactory.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_debug.h>
#include <kis_doc2.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <widgets/kis_multi_integer_filter_widget.h>
#include <kis_selection.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_types.h>

KisRoundCornersFilter::KisRoundCornersFilter() : KisFilter(id(), KisFilter::categoryMap(), i18n("&Round Corners..."))
{
    setSupportsPainting(false);
    setSupportsPreview(true);

}

void KisRoundCornersFilter::process(KisConstProcessingInformation srcInfo,
                                    KisProcessingInformation dstInfo,
                                    const QSize& size,
                                    const KisFilterConfiguration* config,
                                    KoUpdater* progressUpdater
                                   ) const
{

    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_UNUSED(config);
    Q_ASSERT(!src.isNull());
    Q_ASSERT(!dst.isNull());

    if (!src ||
            !dst ||
            !size.isValid() ||
            !config) {
        warnKrita << "Invalid parameters for round corner filter";
        dbgPlugins << src << " " << dst << " " << size << " " << config;
        return;
    }

    //read the filter configuration values from the KisFilterConfiguration object
    qint32 radius = qMax(1, config->getInt("radius" , 30));

    quint32 pixelSize = src->pixelSize();

    if (progressUpdater) {
        progressUpdater->setRange(0, size.height());
    }

    qint32 width = size.width();
    qint32 height = size.height();

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), width, dstInfo.selection());
    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), width, srcInfo.selection());

    const KoColorSpace* cs = src->colorSpace();

    qint32 x0 = dstTopLeft.x();
    qint32 y0 = dstTopLeft.y();
    for (qint32 y = 0; y < size.height(); y++) {
        qint32 x = dstTopLeft.x();
        while (!srcIt.isDone()) {
            if (srcIt.isSelected()) {
                memcpy(dstIt.rawData(), srcIt.oldRawData(), pixelSize);
                if (x <= radius && y <= radius) {
                    double dx = radius - x;
                    double dy = radius - y;
                    double dradius = static_cast<double>(radius);
                    if (dx >= sqrt(dradius*dradius - dy*dy)) {
                        cs->setAlpha(dstIt.rawData(), 0, 1);
                    }
                } else if (x >= x0 + width - radius && y <= radius) {
                    double dx = x + radius - x0 - width;
                    double dy = radius - y;
                    double dradius = static_cast<double>(radius);
                    if (dx >= sqrt(dradius*dradius - dy*dy)) {
                        cs->setAlpha(dstIt.rawData(), 0, 1);
                    }
                } else if (x <= radius && y >= y0 + height - radius) {
                    double dx = radius - x;
                    double dy = y + radius - y0 - height;
                    double dradius = static_cast<double>(radius);
                    if (dx >= sqrt(dradius*dradius - dy*dy)) {
                        cs->setAlpha(dstIt.rawData(), 0, 1);
                    }
                } else if (x >= x0 + width - radius && y >= y0 + height - radius) {

                    double dx = x + radius - x0 - width;
                    double dy = y + radius - y0 - height;
                    double dradius = static_cast<double>(radius);
                    if (dx >= sqrt(dradius*dradius - dy*dy)) {
                        cs->setAlpha(dstIt.rawData(), 0, 1);
                    }
                }
            }
            ++srcIt;
            ++dstIt;
            ++x;
        }
        srcIt.nextRow();
        dstIt.nextRow();
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
