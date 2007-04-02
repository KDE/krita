/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
#include "kis_simple_noise_reducer.h"

#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include <kis_iterators_pixel.h>
#include <kis_autobrush_resource.h>
#include <kis_convolution_painter.h>
#include <kis_global.h>
#include <kis_multi_integer_filter_widget.h>
#include <kis_meta_registry.h>
#include <kis_paint_device.h>

KisSimpleNoiseReducer::KisSimpleNoiseReducer()
    : KisFilter(id(), "enhance", i18n("&Gaussian Noise Reduction"))
{
}


KisSimpleNoiseReducer::~KisSimpleNoiseReducer()
{
}

KisFilterConfigWidget * KisSimpleNoiseReducer::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP)
{
    vKisIntegerWidgetParam param;
    param.push_back( KisIntegerWidgetParam( 0, 255, 50, i18n("Threshold"), "threshold" ) );
    param.push_back( KisIntegerWidgetParam( 0, 10, 1, i18n("Window size"), "windowsize") );
    return new KisMultiIntegerFilterWidget(id().id(), parent, id().id(), param );
}

KisFilterConfiguration * KisSimpleNoiseReducer::designerConfiguration(const KisPaintDeviceSP)
{
    KisFilterConfiguration* config = new KisFilterConfiguration(m_id.id(), 0);
    config->setProperty("threshold", 50);
    config->setProperty("windowsize", 1);
    return config;
}

inline int ABS(int v)
{
    if(v < 0) return -v;
    return v;
}

void KisSimpleNoiseReducer::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* config)
{
    int threshold, windowsize;
    if(config ==0)
    {
        config = defaultConfiguration(src);
    }
    
    threshold = config->getInt("threshold", 50);
    windowsize = config->getInt("windowsize", 1);
    
    KoColorSpace* cs = src->colorSpace();
    
    // Compute the blur mask
    KisAutobrushShape* kas = new KisAutobrushCircleShape(2*windowsize+1, 2*windowsize+1, windowsize, windowsize);
    
    QImage mask;
    kas->createBrush(&mask);
    mask.save("testmask.png", "PNG");
    
    KisKernelSP kernel = KisKernel::fromQImage(mask);
    
    KisPaintDeviceSP interm = new KisPaintDevice(*src);
    KisConvolutionPainter painter( interm );
    painter.beginTransaction("bouuh");
    painter.applyMatrix(kernel, srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height(), BORDER_REPEAT);
    
    if (painter.cancelRequested()) {
        cancel();
    }
    

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width() );
    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width());
    KisHLineConstIteratorPixel intermIt = interm->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width());
    
    for( int j = 0; j < size.height(); j++)
    {
        while( ! srcIt.isDone() )
        {
            if(srcIt.isSelected())
            {
                quint8 diff = cs->difference(srcIt.oldRawData(), intermIt.rawData());
                if( diff > threshold)
                {
                    cs->bitBlt( dstIt.rawData(), 0, cs, intermIt.rawData(), 0, 0, 0, 255, 1, 1, cs->compositeOp(COMPOSITE_COPY) );
                }
            }
            incProgress();
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

