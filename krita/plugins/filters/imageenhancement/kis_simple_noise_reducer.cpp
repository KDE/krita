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

#include <kis_iterators_pixel.h>

#include <kis_autobrush_resource.h>
#include <kis_convolution_painter.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_multi_integer_filter_widget.h>
#include <kis_meta_registry.h>

KisSimpleNoiseReducer::KisSimpleNoiseReducer()
    : KisFilter(id(), "enhance", i18n("&Gaussian Noise Reduction"))
{
}


KisSimpleNoiseReducer::~KisSimpleNoiseReducer()
{
}

KisFilterConfigWidget * KisSimpleNoiseReducer::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP)
{
    vKisIntegerWidgetParam param;
    param.push_back( KisIntegerWidgetParam( 0, 255, 50, i18n("Threshold"), "threshold" ) );
    param.push_back( KisIntegerWidgetParam( 0, 10, 1, i18n("Window size"), "windowsize") );
    return new KisMultiIntegerFilterWidget(parent, id().id().ascii(), id().id().ascii(), param );
}

KisFilterConfiguration* KisSimpleNoiseReducer::configuration(QWidget* nwidget)
{
    KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
    if( widget == 0 )
    {
        return new KisSimpleNoiseReducerConfiguration( 50, 1);
    } else {
        return new KisSimpleNoiseReducerConfiguration( widget->valueAt( 0 ), widget->valueAt( 1 ) );
    }
}

inline int ABS(int v)
{
    if(v < 0) return -v;
    return v;
}

void KisSimpleNoiseReducer::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    int threshold, windowsize;
    if(config !=0)
    {
        KisSimpleNoiseReducerConfiguration* configSNRC = (KisSimpleNoiseReducerConfiguration*)config;
        threshold = configSNRC->threshold();
        windowsize = configSNRC->windowsize();
    } else {
        threshold = 50;
        windowsize = 1;
    }
    
    KisColorSpace* cs = src->colorSpace();
    Q_INT32 depth = cs->nColorChannels();
    
    // Compute the blur mask
    KisAutobrushShape* kas = new KisAutobrushCircleShape(2*windowsize+1, 2*windowsize+1, windowsize, windowsize);
    
    QImage mask;
    kas->createBrush(&mask);
    
    KisKernelSP kernel = KisKernel::fromQImage(mask);
    
    KisPaintDeviceSP interm = new KisPaintDevice(*src);
    KisConvolutionPainter painter( interm );
    painter.beginTransaction("bouuh");
    painter.applyMatrix(kernel, rect.x(), rect.y(), rect.width(), rect.height(), BORDER_REPEAT);
    
    if (painter.cancelRequested()) {
        cancel();
    }
    

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(rect.x(), rect.y(), rect.width(), true );
    KisHLineIteratorPixel srcIt = src->createHLineIterator(rect.x(), rect.y(), rect.width(), false);
    KisHLineIteratorPixel intermIt = interm->createHLineIterator(rect.x(), rect.y(), rect.width(), false);
    
    for( int j = 0; j < rect.height(); j++)
    {
        while( ! srcIt.isDone() )
        {
            if(srcIt.isSelected())
            {
                Q_UINT8 diff = cs->difference(srcIt.oldRawData(), intermIt.rawData());
                if( diff > threshold)
                {
                    cs->bitBlt( dstIt.rawData(), 0, cs, intermIt.rawData(), 0, 0, 0, 255, 1, 1, KisCompositeOp(COMPOSITE_COPY) );
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

