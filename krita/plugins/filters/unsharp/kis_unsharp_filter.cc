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

#include "kis_unsharp_filter.h"

#include <kcombobox.h>
#include <knuminput.h>

#include <kis_autobrush_resource.h>
#include <kis_convolution_painter.h>
#include <kis_iterators_pixel.h>


#include "kis_wdg_unsharp.h"
#include "ui_wdgunsharp.h"

KisUnsharpFilter::KisUnsharpFilter() : KisFilter(id(), "enhance", i18n("&Unsharp Mask..."))
{
}

KisFilterConfigWidget * KisUnsharpFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP )
{
    return new KisWdgUnsharp(this, parent, "configuration of color to alpha");
}

KisFilterConfiguration* KisUnsharpFilter::configuration(QWidget* w)
{
    KisWdgUnsharp * wCTA = dynamic_cast<KisWdgUnsharp*>(w);
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    if(wCTA)
    {
        config->setProperty("halfSize", wCTA->widget()->intHalfSize->value() );
        config->setProperty("amount", wCTA->widget()->doubleAmount->value() );
        config->setProperty("threshold", wCTA->widget()->intThreshold->value() );
    }
    return config;
    return 0;
}

void KisUnsharpFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
//     Q_ASSERT(src != 0);
//     Q_ASSERT(dst != 0);
    
    setProgressTotalSteps(rect.width() * rect.height());

    if(!config) config = new KisFilterConfiguration(id().id(), 1);
    
    QVariant value;
    uint halfSize = (config->getProperty("halfSize", value)) ? value.toUInt() : 4;
    uint size = 2 * halfSize + 1;
    double amount = (config->getProperty("amount", value)) ? value.toDouble() : 0.1;
    uint threshold = (config->getProperty("threshold", value)) ? value.toUInt() : 20;
    
    kdDebug() << " brush size = " << size << " " << halfSize << endl;
    KisAutobrushShape* kas = new KisAutobrushCircleShape(size, size , halfSize, halfSize);
    
    QImage mask;
    kas->createBrush(&mask);
    mask.save("testmask.png", "PNG");
    
    KisKernelSP kernel = KisKernelSP(KisKernel::fromQImage(mask));
    
    KisPaintDeviceSP interm = KisPaintDeviceSP(new KisPaintDevice(*src));
    KoColorSpace * cs = src->colorSpace();

    KisConvolutionPainter painter( interm );
    painter.beginTransaction("bouuh");
    painter.applyMatrix(kernel, rect.x(), rect.y(), rect.width(), rect.height(), BORDER_REPEAT);
    
    if (painter.cancelRequested()) {
        cancel();
    }
    
    KisHLineIteratorPixel dstIt = dst->createHLineIterator(rect.x(), rect.y(), rect.width(), true );
    KisHLineIteratorPixel srcIt = src->createHLineIterator(rect.x(), rect.y(), rect.width(), false);
    KisHLineIteratorPixel intermIt = interm->createHLineIterator(rect.x(), rect.y(), rect.width(), false);

    Q_UINT8 *colors[2];
    
    int pixelsProcessed = 0;
    Q_INT32 weights[2];
    weights[0] = 128;
    Q_INT32 factor = (Q_UINT32) 128 / amount;
    weights[1] = (factor - 128);
    kdDebug() << (int) weights[0] << " " << (int)weights[1] << " " << factor << endl;
    for( int j = 0; j < rect.height(); j++)
    {
        while( ! srcIt.isDone() )
        {
            if(srcIt.isSelected())
            {
                Q_UINT8 diff = cs->difference(srcIt.oldRawData(), intermIt.rawData());
                if( diff > threshold)
                {
                    colors[0] = srcIt.rawData();
                    colors[1] = intermIt.rawData();
                    cs->convolveColors(colors, weights, KoChannelInfo::FLAG_COLOR, dstIt.rawData(),  factor, 0, 2 );
                }
            }
            setProgress(++pixelsProcessed);
            ++srcIt;
            ++dstIt;
            ++intermIt;
        }
        srcIt.nextRow();
        dstIt.nextRow();
        intermIt.nextRow();
    }


    setProgressDone(); // Must be called even if you don't really support progression
}
