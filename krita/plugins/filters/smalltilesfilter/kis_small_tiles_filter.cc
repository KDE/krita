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
#include <q3valuevector.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_painter.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_progress_display_interface.h>
#include <kis_paint_device.h>
#include <kis_filter_strategy.h>
#include <kis_painter.h>
#include <kis_selection.h>

#include "kis_multi_integer_filter_widget.h"
#include "kis_small_tiles_filter.h"


#define MIN(a,b) (((a)<(b))?(a):(b))

void KisSmallTilesFilterConfiguration::fromXML(const QString & s)
{
    KisFilterConfiguration::fromXML(s);
    m_numberOfTiles = getInt("numberOfTiles");
}

QString KisSmallTilesFilterConfiguration::toString()
{
    m_properties.clear();
    setProperty("numberOfTiles()", m_numberOfTiles);

    return KisFilterConfiguration::toString();
}

KisSmallTilesFilter::KisSmallTilesFilter() : KisFilter(id(), "map", i18n("&Small Tiles..."))
{
}

void KisSmallTilesFilter::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* configuration)
{
        //read the filter configuration values from the KisFilterConfiguration object
        quint32 numberOfTiles = ((KisSmallTilesFilterConfiguration*)configuration)->numberOfTiles();

/*        createSmallTiles(src, dst, rect, numberOfTiles);
}

void KisSmallTilesFilter::createSmallTiles(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect& rect, quint32 numberOfTiles)
{*/
    if (!src) return;
    if (!dst) return;

    QRect srcRect = src->exactBounds();

    int w = static_cast<int>(srcRect.width() / numberOfTiles);
    int h = static_cast<int>(srcRect.height() / numberOfTiles);

    KisPaintDeviceSP tile = KisPaintDeviceSP(0);
    if (src->hasSelection()) {
        KisPaintDeviceSP tmp = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace(), "selected bit"));
        KisPainter gc(tmp);
        gc.bltSelection(0, 0, COMPOSITE_COPY, src, OPACITY_OPAQUE, srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height());
        tile = tmp->createThumbnailDevice(srcRect.width() / numberOfTiles, srcRect.height() / numberOfTiles);
    }
    else {
        tile = src->createThumbnailDevice(srcRect.width() / numberOfTiles, srcRect.height() / numberOfTiles);
    }
    if (tile.isNull()) return;

    KisPaintDeviceSP scratch = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace()));

    KisPainter gc(scratch);

    setProgressTotalSteps(numberOfTiles);

    for (uint y = 0; y < numberOfTiles; ++y) {
        for (uint x = 0; x < numberOfTiles; ++x) {
            // XXX make composite op and opacity configurable
            gc.bitBlt( w * x, h * y, COMPOSITE_COPY, tile, 0, 0, w, h);
            setProgress(y);
        }
    }
    gc.end();

    gc.begin(dst);
    if (src->hasSelection()) {
        gc.bltSelection(dstTopLeft.x(), dstTopLeft.y(), COMPOSITE_OVER, scratch, src->selection(), OPACITY_OPAQUE, 0, 0, size.width(), size.height() );
    }
    else {
        gc.bitBlt(dstTopLeft.x(), dstTopLeft.y(), COMPOSITE_OVER, scratch, OPACITY_OPAQUE, 0, 0, size.width(), size.height() );
    }
    setProgressDone();
    gc.end();

    setProgressDone();
}

KisFilterConfigWidget * KisSmallTilesFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP /*dev*/)
{
    vKisIntegerWidgetParam param;
    param.push_back( KisIntegerWidgetParam( 2, 5, 1, i18n("Number of tiles"), "smalltiles" ) );
    return new KisMultiIntegerFilterWidget(parent, id().id().toAscii(), id().id(), param );
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
