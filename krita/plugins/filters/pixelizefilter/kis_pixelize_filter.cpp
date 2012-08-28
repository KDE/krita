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

#include "kis_pixelize_filter.h"


#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>
//Added by qt3to4:
#include <QVector>

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kpluginfactory.h>
#include <knuminput.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_debug.h>
#include <kis_doc2.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "widgets/kis_multi_integer_filter_widget.h"
#include <kis_iterator_ng.h>


KisPixelizeFilter::KisPixelizeFilter() : KisFilter(id(), KisFilter::categoryArtistic(), i18n("&Pixelize..."))
{
    setSupportsPainting(true);
}

void KisPixelizeFilter::process(KisPaintDeviceSP device,
                         const QRect& applyRect,
                         const KisFilterConfiguration* configuration,
                         KoUpdater* progressUpdater
                               ) const
{
    QPoint srcTopLeft = applyRect.topLeft();
    Q_ASSERT(device);
    Q_ASSERT(configuration);

    qint32 width = applyRect.width();
    qint32 height = applyRect.height();

    //read the filter configuration values from the KisFilterConfiguration object
    quint32 pixelWidth = configuration->getInt("pixelWidth", 10);
    quint32 pixelHeight = configuration->getInt("pixelHeight", 10);
    if (pixelWidth == 0) pixelWidth = 1;
    if (pixelHeight == 0) pixelHeight = 1;

    qint32 pixelSize = device->pixelSize();
    QVector<qint32> average(pixelSize);

    qint32 count;

    if (progressUpdater) {
        progressUpdater->setRange(0, applyRect.width() * applyRect.height());
    }

    qint32 numberOfPixelsProcessed = 0;

    for (qint32 y = 0; y < height; y += pixelHeight - (y % pixelHeight)) {
        qint32 h = pixelHeight;
        h = qMin(h, height - y);

        for (qint32 x = 0; x < width; x += pixelWidth - (x % pixelWidth)) {
            qint32 w = pixelWidth;
            w = qMin(w, width - x);

            for (qint32 i = 0; i < pixelSize; i++) {
                average[i] = 0;
            }
            count = 0;

            //read
            KisRectConstIteratorSP srcIt = device->createRectConstIteratorNG(srcTopLeft.x() + x, srcTopLeft.y() + y, w, h);
            do {
                for (qint32 i = 0; i < pixelSize; i++) {
                    average[i] += srcIt->oldRawData()[i];
                }
                count++;
            } while (srcIt->nextPixel());

            //average
            if (count > 0) {
                for (qint32 i = 0; i < pixelSize; i++)
                    average[i] /= count;
            }
            //write
            KisRectIteratorSP dstIt = device->createRectIteratorNG(srcTopLeft.x() + x, srcTopLeft.y() + y, w, h);
            do {
                for (int i = 0; i < pixelSize; i++) {
                    dstIt->rawData()[i] = average[i];
                }
            } while (dstIt->nextPixel());
            if (progressUpdater) progressUpdater->setValue(++numberOfPixelsProcessed);
        }
    }
}

KisConfigWidget * KisPixelizeFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(2, 40, 10, i18n("Pixel width"), "pixelWidth"));
    param.push_back(KisIntegerWidgetParam(2, 40, 10, i18n("Pixel height"), "pixelHeight"));
    return new KisMultiIntegerFilterWidget(id().id(),  parent,  id().id(),  param);
}

KisFilterConfiguration* KisPixelizeFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("pixelize", 1);
    config->setProperty("pixelWidth", 10);
    config->setProperty("pixelHeight", 10);
    return config;
}
