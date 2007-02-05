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
    return new KisWdgUnsharp(this, parent);
}

KisFilterConfiguration* KisUnsharpFilter::designerConfiguration(KisPaintDeviceSP)
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("halfSize", 5 );
    config->setProperty("amount", 0.5 );
    config->setProperty("threshold", 10 );
    return config;
}

void KisUnsharpFilter::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& areaSize, KisFilterConfiguration* config)
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    setProgressTotalSteps(areaSize.width() * areaSize.height());

    if(!config) config = new KisFilterConfiguration(id().id(), 1);

    QVariant value;
    uint halfSize = (config->getProperty("halfSize", value)) ? value.toUInt() : 5;
    uint size = 2 * halfSize + 1;
    double amount = (config->getProperty("amount", value)) ? value.toDouble() : 0.5;
    uint threshold = (config->getProperty("threshold", value)) ? value.toUInt() : 10;

//     kDebug() << " brush size = " << size << " " << halfSize << endl;
    KisAutobrushShape* kas = new KisAutobrushCircleShape(size, size , halfSize, halfSize);

    QImage mask;
    kas->createBrush(&mask);
    mask.save("testmask.png", "PNG");

    KisKernelSP kernel = KisKernelSP(KisKernel::fromQImage(mask));

    KisPaintDeviceSP interm = KisPaintDeviceSP(new KisPaintDevice(*src));
    KoColorSpace * cs = src->colorSpace();

    KisConvolutionPainter painter( interm );
    painter.beginTransaction("bouuh");
    painter.applyMatrix(kernel, srcTopLeft.x(), srcTopLeft.y(), areaSize.width(), areaSize.height(), BORDER_REPEAT);

    if (painter.cancelRequested()) {
        cancel();
    }

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), areaSize.width());
    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), areaSize.width());
    KisHLineConstIteratorPixel intermIt = interm->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), areaSize.width());

    int cdepth = cs -> pixelSize();
    quint8 *colors[2];
    colors[0] = new quint8[cdepth];
    colors[1] = new quint8[cdepth];

    int pixelsProcessed = 0;
    qint32 weights[2];
/*    weights[0] = 128;
    qint32 factor = (quint32) 128 / amount;
    weights[1] = (factor - 128);*/
    qint32 factor = 128;
    // XXX: Added static cast to avoid warning
    weights[0] = static_cast<qint32>(factor * ( 1. + amount));
    weights[1] = static_cast<qint32>(-factor * amount);
//     kDebug() << (int) weights[0] << " " << (int)weights[1] << " " << factor << endl;
    for( int j = 0; j < areaSize.height(); j++)
    {
        while( ! srcIt.isDone() )
        {
            if(srcIt.isSelected())
            {
                quint8 diff = cs->difference(srcIt.oldRawData(), intermIt.rawData());
                if( diff > threshold)
                {
                    memcpy(colors[0],srcIt.rawData(), cdepth);
                    memcpy(colors[1],intermIt.rawData(), cdepth);
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
    delete colors[0];
    delete colors[1];


    setProgressDone(); // Must be called even if you don't really support progression
}
