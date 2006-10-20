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

#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>
//Added by qt3to4:
#include <Q3MemArray>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_progress_display_interface.h>

#include "kis_multi_integer_filter_widget.h"
#include "kis_pixelize_filter.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

KisPixelizeFilter::KisPixelizeFilter() : KisFilter(id(), "artistic", i18n("&Pixelize..."))
{
}

void KisPixelizeFilter::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* configuration)
{
// FIXME THIS FILTER DOESN'T WORK WELL IF SRC != DST !!!
    Q_ASSERT( src );
    Q_ASSERT( dst );
    Q_ASSERT( configuration );
    Q_ASSERT( size.isValid() );

    qint32 width = size.width();
    qint32 height = size.height();

    //read the filter configuration values from the KisFilterConfiguration object
    quint32 pixelWidth = ((KisPixelizeFilterConfiguration*)configuration)->pixelWidth();
    quint32 pixelHeight = ((KisPixelizeFilterConfiguration*)configuration)->pixelHeight();

    qint32 pixelSize = src->pixelSize();
    Q3MemArray<qint32> average(  pixelSize );

    qint32 count;

    //calculate the total number of pixels
    qint32 numX=0;
    qint32 numY=0;

    for (qint32 x = srcTopLeft.x(); x < srcTopLeft.x() + width; x += pixelWidth - (x % pixelWidth))
    {
        numX++;
    }
    for (qint32 y = srcTopLeft.y(); y < srcTopLeft.y() + height; y += pixelHeight - (y % pixelHeight))
    {
        numY++;
    }

    setProgressTotalSteps( numX * numY );
    setProgressStage(i18n("Applying pixelize filter..."),0);

    qint32 numberOfPixelsProcessed = 0;

    for (qint32 y = 0; y < height; y += pixelHeight - ((srcTopLeft.y() + y) % pixelHeight))
    {
        qint32 h = pixelHeight - ( ( srcTopLeft.y() + y) % pixelHeight);
        h = MIN(h, height - y);

        for (qint32 x = 0; x < width; x += pixelWidth - ( (srcTopLeft.x() + x) % pixelWidth))
        {
            qint32 w = pixelWidth - ( (srcTopLeft.x() + x) % pixelWidth);
            w = MIN(w, width - x);

            for (qint32 i = 0; i < pixelSize; i++)
            {
                average[i] = 0;
            }
            count = 0;

            //read
            KisRectConstIteratorPixel srcIt = src->createRectConstIterator(srcTopLeft.x() + x, srcTopLeft.y() + y, w, h);
            while( ! srcIt.isDone() ) {
                if(srcIt.isSelected())
                {
                    for (qint32 i = 0; i < pixelSize; i++)
                    {
                        average[i] += srcIt.oldRawData()[i];
                    }
                    count++;
                }
                ++srcIt;
            }

            //average
            if (count > 0)
            {
                for (qint32 i = 0; i < pixelSize; i++)
                    average[i] /= count;
            }
            //write
            srcIt = src->createRectConstIterator(srcTopLeft.x() + x, srcTopLeft.y() + y, w, h);
            KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x() + x, dstTopLeft.y() + y, w, h );
            while( ! srcIt.isDone() )
            {
                if(srcIt.isSelected())
                {
                    for( int i = 0; i < pixelSize; i++)
                    {
                        dstIt.rawData()[i] = average[i];
                    }
                }
                ++srcIt;
                ++dstIt;
            }
            numberOfPixelsProcessed++;
            setProgress(numberOfPixelsProcessed);
        }
    }

    setProgressDone();
}

KisFilterConfigWidget * KisPixelizeFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP /*dev*/)
{
    vKisIntegerWidgetParam param;
    param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Pixel width"), "pixelWidth" ) );
    param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Pixel height"), "pixelHeight" ) );
    return new KisMultiIntegerFilterWidget(parent, id().id().toAscii(), id().id(), param );
}

KisFilterConfiguration* KisPixelizeFilter::configuration(QWidget* nwidget)
{
    KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
    if( widget == 0 )
    {
        return new KisPixelizeFilterConfiguration( 10, 10);
    } else {
        return new KisPixelizeFilterConfiguration( widget->valueAt( 0 ), widget->valueAt( 1 ) );
    }
}

KisFilterConfiguration * KisPixelizeFilter::configuration()
{
    return new KisPixelizeFilterConfiguration(10, 10);
}
