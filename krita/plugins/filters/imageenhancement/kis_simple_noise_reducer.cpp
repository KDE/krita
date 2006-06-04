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
#include "kis_multi_integer_filter_widget.h"
#include <kis_meta_registry.h>
#include <KoColorSpaceFactoryRegistry.h>

KisSimpleNoiseReducer::KisSimpleNoiseReducer()
    : KisFilter(id(), "enhance", i18n("&Simple Noise Reduction"))
{
}


KisSimpleNoiseReducer::~KisSimpleNoiseReducer()
{
}

KisFilterConfigWidget * KisSimpleNoiseReducer::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP)
{
    vKisIntegerWidgetParam param;
    param.push_back( KisIntegerWidgetParam( 0, 100, 50, i18n("Threshold"), "threshold" ) );
    param.push_back( KisIntegerWidgetParam( 0, 10, 1, i18n("Window size"), "windowsize") );
    return new KisMultiIntegerFilterWidget(parent, id().id().toAscii(), id().id(), param );
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
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

    qint32 depth = src->colorSpace()->nColorChannels();
    QRect extends = src->exactBounds();
    int lastx = extends.width() - windowsize;
    int lasty = extends.height() - windowsize;
    int* means = new int[depth];

    int pixelsProcessed = 0;
    setProgressTotalSteps(rect.width() * rect.height());
    while( ! srcIt.isDone() )
    {
        if(srcIt.isSelected())
        {
            int x = srcIt.x();
            int y = srcIt.y();
            int lx = ( x >= lastx ) ? 2 * windowsize - (x - lastx ) : 2 * windowsize + 1;
            int ly = ( y >= lasty ) ? 2 * windowsize - (y - lasty ) : 2 * windowsize + 1;
            if(x > windowsize) x -= windowsize;
            else x = 0;
            if(y > windowsize) y -= windowsize;
            else y = 0;
            KisRectIteratorPixel neighbourgh_srcIt = src->createRectIterator(x, y, lx, ly, false);
            // Reinit means
            for( int i = 0; i < depth; i++)
            {
                means[i] = 0;
            }
            while( ! neighbourgh_srcIt.isDone() )
            {
                if(neighbourgh_srcIt.x() != srcIt.x() || neighbourgh_srcIt.y() != srcIt.y() )
                {
                    for( int i = 0; i < depth; i++)
                    {
                        means[i] += neighbourgh_srcIt.oldRawData()[i];
                    }
                }
                ++neighbourgh_srcIt;
            }

            // Count the number of time that the data is too much different from is neighbourgh
            int pixelsnb = lx * ly - 1;

            if (pixelsnb != 0) {
                int depthbad = 0;
                for( int i = 0; i < depth; i++)
                {
                    means[i] /= pixelsnb;
                    if( 100*ABS(means[i] - srcIt.oldRawData()[i]) > threshold * means[i] )
                    {
                        ++depthbad;
                    }
                }
                // Change the value of the pixel, if the pixel is too much different
                if(depthbad > depth / 2)
                {
                    for( int i = 0; i < depth; i++)
                    {
                        dstIt.rawData()[i] = means[i];
                    }
                }
            } else {
                kDebug() << "pixelsnb == 0: lx " << lx << ", " << ly << "\n";
            }

        }
        setProgress(++pixelsProcessed);
        ++srcIt;
        ++dstIt;
    }
    setProgressDone(); // Must be called even if you don't really support progression
}

