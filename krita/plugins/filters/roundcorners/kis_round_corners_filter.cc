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
#include <math.h>

#include <QPoint>
#include <QSpinBox>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_progress_display_interface.h>

#include "kis_multi_integer_filter_widget.h"
#include "kis_round_corners_filter.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

KisRoundCornersFilter::KisRoundCornersFilter() : KisFilter(id(), "map", i18n("&Round Corners..."))
{
}

void KisRoundCornersFilter::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* configuration)
{
    //read the filter configuration values from the KisFilterConfiguration object
    qint32 radius = (qint32)((KisRoundCornersFilterConfiguration*)configuration)->radius();
    quint32 pixelSize = src->pixelSize();

    setProgressTotalSteps( size.height() );
    setProgressStage(i18n("Applying pixelize filter..."),0);

    qint32 width = size.width();
    qint32 height = size.height();

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(srcTopLeft.x(), srcTopLeft.y(), width );
    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(dstTopLeft.x(), dstTopLeft.y(), width);

    for (qint32 y = 0; y < size.height(); y++)
    {
        qint32 x = dstTopLeft.x();
        qint32 x0 = dstTopLeft.x();
        qint32 y0 = dstTopLeft.x();
        while( ! srcIt.isDone() )
        {
            if(srcIt.isSelected())
            {
                for( unsigned int i = 0; i < pixelSize; i++)
                {
                    if ( i < pixelSize - 1 )
                    {
                        dstIt.rawData()[i] = srcIt.oldRawData()[i];
                    }
                    else
                    {
                        if( x <= radius && y <= radius)
                        {
                            double dx = radius - x;
                            double dy = radius - y;
                            double dradius = static_cast<double>(radius);
                            if ( dx >= sqrt( dradius*dradius - dy*dy ) )
                            {
                                dstIt.rawData()[i] = 0;
                            }
                        }
                        else if( x >= x0 + width - radius && y <= radius)
                        {
                            double dx = x + radius - x0 - width;
                            double dy = radius - y;
                            double dradius = static_cast<double>(radius);
                            if ( dx >= sqrt( dradius*dradius - dy*dy ) )
                            {
                                dstIt.rawData()[i] = 0;
                            }
                        }
                        else if( x <= radius && y >= y0 + height - radius)
                        {
                            double dx = radius - x;
                            double dy = y + radius - y0 - height;
                            double dradius = static_cast<double>(radius);
                            if ( dx >= sqrt( dradius*dradius - dy*dy ) )
                            {
                                dstIt.rawData()[i] = 0;
                            }
                        }
                        else if( x >= x0 + width - radius && y >= y0 + height - radius)
                        {

                            double dx = x + radius - x0 - width;
                            double dy = y + radius - y0 - height;
                            double dradius = static_cast<double>(radius);
                            if ( dx >= sqrt( dradius*dradius - dy*dy ) )
                            {
                                dstIt.rawData()[i] = 0;
                            }
                        }
                    }
                }
            }
            ++srcIt;
            ++dstIt;
            ++x;
        }
        srcIt.nextRow();
        dstIt.nextRow();
        setProgress(y);
    }
    setProgressDone();
}

KisFilterConfigWidget * KisRoundCornersFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP /*dev*/)
{
    vKisIntegerWidgetParam param;
    param.push_back( KisIntegerWidgetParam( 2, 100, 30, i18n("Radius"), "radius" ) );
    return new KisMultiIntegerFilterWidget(parent, id().id().toAscii(), id().id(), param );
}

KisFilterConfiguration* KisRoundCornersFilter::configuration(QWidget* nwidget)
{
    KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
    if( widget == 0 )
    {
        return new KisRoundCornersFilterConfiguration( 30 );
    } else {
        return new KisRoundCornersFilterConfiguration( widget->valueAt( 0 ) );
    }
}
