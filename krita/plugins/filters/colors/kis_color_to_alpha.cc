/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_color_to_alpha.h"

#include <qcheckbox.h>
#include <qspinbox.h>

#include <kcolorbutton.h>

#include <KoProgressUpdater.h>

#include <kis_iterators_pixel.h>
#include <kis_paint_device.h>
#include <kis_selection.h>

#include "ui_wdgcolortoalphabase.h"
#include "kis_wdg_color_to_alpha.h"

KisFilterColorToAlpha::KisFilterColorToAlpha() : KisFilter(id(), CategoryColors, i18n("&Color to Alpha..."))
{
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
    setSupportsAdjustmentLayers(false);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisFilterConfigWidget * KisFilterColorToAlpha::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP )
{
    return new KisWdgColorToAlpha(this, parent);
}

KisFilterConfiguration* KisFilterColorToAlpha::factoryConfiguration(KisPaintDeviceSP )
{
    KisFilterConfiguration* config = new KisFilterConfiguration("colortoalpha", 1);
    config->setProperty("targetcolor", QColor(255,255,255) );
    config->setProperty("threshold", 0);
    return config;
}

void KisFilterColorToAlpha::process(KisFilterConstantProcessingInformation srcInfo,
                 KisFilterProcessingInformation dstInfo,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater
        ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    if(config == 0) config = new KisFilterConfiguration("colortoalpha", 1);

    QVariant value;
    QColor cTA = (config->getProperty("targetcolor", value)) ? value.value<QColor>() : QColor(255,255,255);
    int threshold = (config->getProperty("threshold", value)) ? value.toInt() : 0;

    KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height(), dstInfo.selection() );
    KisRectConstIteratorPixel srcIt = src->createRectConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height(), srcInfo.selection() );

    int pixelsProcessed = 0;
    int totalCost = size.width() * size.height() / 100;
    int currentProgress = 0;

    KoColorSpace * cs = src->colorSpace();
    qint32 pixelsize = cs->pixelSize();

    quint8* color = new quint8[pixelsize];
    cs->fromQColor(cTA, color);

    while( ! srcIt.isDone() )
    {
        if(srcIt.isSelected())
        {
            quint8 d = cs->difference(color, srcIt.oldRawData());
            if( d >= threshold )
            {
                    cs->setAlpha(dstIt.rawData(), 255, 1);
            } else {
                cs->setAlpha(dstIt.rawData(), (255 * d ) / threshold, 1 );
            }
        }
        progressUpdater->setProgress((++currentProgress) / totalCost);
        ++srcIt;
        ++dstIt;
    }
    delete[] color;
}
