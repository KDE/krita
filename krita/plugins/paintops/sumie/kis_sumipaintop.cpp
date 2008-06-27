/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_sumipaintop.h"
#include <cmath>

#include <QRect>
#include <QColor>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_debug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_random_accessor.h"

#include "kis_datamanager.h"

#include "stroke.h"

KisPaintOp * KisSumiPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image)
{
    Q_UNUSED( settings );
    KisPaintOp * op = new KisSumiPaintOp(painter, image);
	Q_CHECK_PTR(op);
    return op;
}


KisSumiPaintOp::KisSumiPaintOp(KisPainter * painter, KisImageSP image)
    : KisPaintOp(painter)
{
    dbgKrita << "START OF KisSumiPaintOp" << endl;
    newStrokeFlag = true;
	m_image = image;
}

KisSumiPaintOp::~KisSumiPaintOp()
{
	dbgKrita << "END OF KisSumiPaintOp" << endl;
}

void KisSumiPaintOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return;
    // read, write pixel data
    KisPaintDeviceSP device = painter()->device();
    if (!device) return;

    qint32 x = (qint32)info.pos().x();
    qint32 y = (qint32)info.pos().y();

	dbgPlugins << "X:" << x << endl;
	dbgPlugins << "Y:" << y << endl;

	QRect layerRect = m_image->bounds();
//	int w = layerRect.width();
	c = Qt::black;    
/*	KisRandomAccessor accessor = dab->createRandomAccessor(x, y);
	quint8 *pixel = accessor.rawData();
	accessor.moveTo(x,y);
	dab->colorSpace()->fromQColor(c, pixel);*/

    if ( newStrokeFlag ) {
		stroke.x1 = info.pos().x();
		stroke.y1 = info.pos().y();
		KoColor color = painter()->paintColor();
		stroke.setColor(color);
		dbgKrita << "Everything Ok?" << flush << endl;
        newStrokeFlag = false;
    } else
	{	
        stroke.x1 = stroke.x2;
        stroke.y1 = stroke.y2;
		stroke.x2 = info.pos().x();
		stroke.y2 = info.pos().y();

		dab = cachedDab( );
		stroke.draw(dab);
		QRect rc = dab->extent();
	    painter()->bitBlt( rc.topLeft(), dab, rc );

	}



/*    QRect rc = dab->extent();
    painter()->bitBlt( rc.topLeft(), dab, rc );*/
    //painter()->bltSelection(x, y, painter()->compositeOp(), dab, painter()->opacity(), x, y, 1, 1);
}

