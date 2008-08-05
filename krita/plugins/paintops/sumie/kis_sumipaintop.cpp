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
#include "kis_sumipaintopsettings.h"

#include <cmath>

#include <QRect>
#include <QList>
#include <QColor>
#include <QMutexLocker>

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

#include "lines.h"
#include "brush.h"
#include "brush_shape.h"

KisPaintOp * KisSumiPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image = 0)
{
    const KisSumiPaintOpSettings *sumiSettings = dynamic_cast<const KisSumiPaintOpSettings *>(settings.data());
    Q_ASSERT(settings == 0 || sumiSettings != 0);

	KisPaintOp * op = new KisSumiPaintOp(sumiSettings, painter, image);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisSumiPaintOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image)
{
    Q_UNUSED( inputDevice );
    Q_UNUSED( image );
    return new KisSumiPaintOpSettings( parent );
}

KisPaintOpSettingsSP KisSumiPaintOpFactory::settings(KisImageSP image)
{
    Q_UNUSED( image );
    return new KisSumiPaintOpSettings( 0 );
}

KisSumiPaintOp::KisSumiPaintOp(const KisSumiPaintOpSettings *settings,KisPainter * painter, KisImageSP image)
    : KisPaintOp(painter)
{
    newStrokeFlag = true;
    m_image = image;
	BrushShape brushShape;

	if (settings->brushDimension() == 1){
		brushShape.fromLine(settings->radius(), settings->sigma() );
	}
	else if (settings->brushDimension() == 2)
	{
		brushShape.fromGaussian(settings->radius(), settings->sigma() );
	}
	else {
		Q_ASSERT(false);
	}

	m_brush.setBrushShape(brushShape);

	m_brush.enableMousePressure( settings->mousePressure() );

	m_brush.setInkDepletion( settings->curve() );
	m_brush.setInkColor( painter->paintColor() );

    m_brush.setScale(settings->scaleFactor() );
    m_brush.setRandom(settings->randomFactor() );
    m_brush.setShear(settings->shearFactor() );
	// delete??
}

KisSumiPaintOp::~KisSumiPaintOp()
{
}

void KisSumiPaintOp::paintAt(const KisPaintInformation& info)
{
	Q_UNUSED(info);
}


double KisSumiPaintOp::paintLine(const KisPaintInformation &pi1,const KisPaintInformation &pi2,double savedDist ){
    QMutexLocker locker(&m_mutex);

	Q_UNUSED(savedDist);

    if (!painter()) return -1;
    //color: painter()->paintColor()
	KisPaintDeviceSP device = painter()->device();
    if (!device) return -1;

//     if ( newStrokeFlag ) {
//         newStrokeFlag = false;
//     } else
//     {

	dab = cachedDab( );
	dab->clear();

	Lines line;
	//line.drawDDAALine(dab, pi1.pos().x(), pi1.pos().y(), pi2.pos().x(), pi2.pos().y(), painter()->paintColor());
	//line.drawThickLine(dab, pi1.pos().x(), pi1.pos().y(), pi2.pos().x(), pi2.pos().y(),painter()->paintColor(),painter()->paintColor(), 2*pi1.pressure(), 20*pi2.pressure() );
	//line.drawWuLine(dab, pi1.pos().x(), pi1.pos().y(), pi2.pos().x(), pi2.pos().y(), painter()->paintColor());

// testing lines in every direction (circle of lines)
/*	int x, y;
	float phase = 0.0;
	
for (float theta= phase; theta<360+phase; theta += 10 )
	{
		x = (int)(100.0*cos(theta*3.14/180.0)+pi1.pos().x());
		y = (int)(-100.0*sin(theta*3.14/180.0)+pi1.pos().y());
// 		line.drawDDAALine(dab, x, y, pi1.pos().x(), pi1.pos().y(), painter()->paintColor());
 		line.drawThickLine(dab, x, y, pi1.pos().x(), pi1.pos().y(),painter()->paintColor(),painter()->paintColor(), 1, 1);
// 		line.drawWuLine(dab, x,y ,pi1.pos().x(), pi1.pos().y(), painter()->paintColor());
		//painter()->drawThickLine(QPointF(x, y), QPointF(pi1.pos().x(), pi1.pos().y()), 1, 1);
		
	}

phase = 0.0;
for (float theta= phase; theta<360+phase; theta += 10 )
	{
		x = (int)(100.0*cos(theta*3.14/180.0)+pi2.pos().x());
		y = (int)(-100.0*sin(theta*3.14/180.0)+pi2.pos().y());
		//line.drawWuLine(dab, x, y, pi2.pos().x(), pi2.pos().y(), painter()->paintColor());
		//line.drawDDAALine(dab, x, y, pi2.pos().x(), pi2.pos().y(), painter()->paintColor());
	}

*/
//     dbgPlugins << "Start coords:";
// 
//  	dbgPlugins << pi1.pos().x();
//     dbgPlugins << pi1.pos().y();
//     dbgPlugins << "End coords:";
//     dbgPlugins << pi2.pos().x();
//     dbgPlugins << pi2.pos().y();
//     dbgPlugins << "--------------";

    m_brush.paintLine(dab, pi1, pi2);

	QRect rc = dab->extent();
	painter()->bitBlt( rc.topLeft(), dab, rc );

//     } newStroke
    //painter()->bltSelection(x, y, painter()->compositeOp(), dab, painter()->opacity(), x, y, 1, 1);
  return 0;
}
