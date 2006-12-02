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

#include <kis_iterators_pixel.h>
#include <kis_paint_device.h>

#include "ui_wdgcolortoalphabase.h"
#include "kis_wdg_color_to_alpha.h"

KisFilterColorToAlpha::KisFilterColorToAlpha() : KisFilter(id(), "colors", i18n("&Color to Alpha..."))
{
}

KisFilterConfigWidget * KisFilterColorToAlpha::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP )
{
    return new KisWdgColorToAlpha(this, parent);
}

KisFilterConfiguration* KisFilterColorToAlpha::designerConfiguration(KisPaintDeviceSP )
{
    KisFilterConfiguration* config = new KisFilterConfiguration("colortoalpha", 1);
    config->setProperty("targetcolor", QColor(255,255,255) );
    config->setProperty("threshold", 0);
    return config;
}

void KisFilterColorToAlpha::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* config)
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    
    if(config == 0) config = new KisFilterConfiguration("colortoalpha", 1);
    
    QVariant value;
    QColor cTA = (config->getProperty("targetcolor", value)) ? value.value<QColor>() : QColor(255,255,255);
    int threshold = (config->getProperty("threshold", value)) ? value.toInt() : 0;
    
    KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height() );
    KisRectConstIteratorPixel srcIt = src->createRectConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height());

    int pixelsProcessed = 0;
    setProgressTotalSteps(size.width() * size.height());

    KoColorSpace * cs = src->colorSpace();
    Q_INT32 pixelsize = cs->pixelSize();
    
    Q_UINT8* color = new Q_UINT8[pixelsize];
    cs->fromQColor(cTA, color);
    
    while( ! srcIt.isDone() )
    {
        if(srcIt.isSelected())
        {
            Q_UINT8 d = cs->difference(color, srcIt.oldRawData());
            if( d >= threshold )
            {
                    cs->setAlpha(dstIt.rawData(), 255, 1);
            } else {
                cs->setAlpha(dstIt.rawData(), (255 * d ) / threshold, 1 );
            }
        }
        setProgress(++pixelsProcessed);
        ++srcIt;
        ++dstIt;
    }
    delete[] color;
    setProgressDone(); // Must be called even if you don't really support progression
}
