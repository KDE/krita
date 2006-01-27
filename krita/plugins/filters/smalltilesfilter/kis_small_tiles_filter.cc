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

#include <qpoint.h>
#include <qspinbox.h>
#include <qvaluevector.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
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
#include <kis_paint_device.h>
#include <kis_filter_strategy.h>
#include <kis_scale_visitor.h>

#include "kis_multi_integer_filter_widget.h"
#include "kis_small_tiles_filter.h"


#define MIN(a,b) (((a)<(b))?(a):(b))

KisSmallTilesFilter::KisSmallTilesFilter() : KisFilter(id(), "map", i18n("&Small Tiles..."))
{
}

void KisSmallTilesFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect)
{
        Q_INT32 x = rect.x(), y = rect.y();
        Q_INT32 width = rect.width();
        Q_INT32 height = rect.height();

        //read the filter configuration values from the KisFilterConfiguration object
        Q_UINT32 numberOfTiles = ((KisSmallTilesFilterConfiguration*)configuration)->numberOfTiles();

        createSmallTiles(src, dst, rect, numberOfTiles);
}

void KisSmallTilesFilter::createSmallTiles(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect& rect, Q_UINT32 numberOfTiles)
{
    Q_INT32 depth = src -> colorSpace() -> nColorChannels();
    KisPaintDeviceSP tmp = new KisPaintDevice( *(src.data()) );

    KisScaleWorker worker(tmp, 1.0 / static_cast<double>(numberOfTiles), 1.0 / static_cast<double>(numberOfTiles), new KisMitchellFilterStrategy() );
    worker.run();

    QRect tmpRect = tmp -> exactBounds();

    for( Q_UINT32 i=0; i < numberOfTiles; i++ )
    {
        for( Q_UINT32 j=0; j < numberOfTiles; j++ )
        {
            for( Q_UINT32 row = tmpRect.y(); row < tmpRect.height(); row++ )
            {
                KisHLineIteratorPixel tmpIt = tmp -> createHLineIterator(tmpRect.x(), row, tmpRect.width() , false);
                KisHLineIteratorPixel dstIt = dst -> createHLineIterator( tmpRect.x() + i * tmpRect.width(), row + j * tmpRect.height(), tmpRect.width() , true);

                while( ! tmpIt.isDone() )
                {
                    if(tmpIt.isSelected())
                    {
                        for( int i = 0; i < depth; i++)
                        {
                            dstIt.rawData()[i] = tmpIt.oldRawData()[i];
                        }
                    }
                    ++tmpIt;
                    ++dstIt;
                }
            }
        }
    }

    setProgressDone();
}

KisFilterConfigWidget * KisSmallTilesFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev)
{
    vKisIntegerWidgetParam param;
    param.push_back( KisIntegerWidgetParam( 2, 5, 1, i18n("Number of tiles") ) );
    return new KisMultiIntegerFilterWidget(parent, id().id().ascii(), id().id().ascii(), param );
}

KisFilterConfiguration* KisSmallTilesFilter::configuration(QWidget* nwidget)
{
    KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
    if( widget == 0 )
    {
        return new KisSmallTilesFilterConfiguration( 2 );
    } else {
        return new KisSmallTilesFilterConfiguration( widget->valueAt( 0 ) );
    }
}
