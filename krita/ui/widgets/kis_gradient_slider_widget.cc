/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
 
#include "kis_gradient_slider_widget.h"

#include <qpainter.h>

#include <kdebug.h>

#include "kis_types.h"
#include "kis_gradient_painter.h"
#include "kis_paint_device.h"
#include "kis_colorspace_registry.h"

#include "kis_autogradient.h"

KisGradientSliderWidget::KisGradientSliderWidget(QWidget *parent, const char* name, WFlags f ) : QWidget( parent, name, f)
{
	setMinimumHeight(20);
	setMaximumHeight(20);
}

void KisGradientSliderWidget::paintEvent ( QPaintEvent* pe )
{
	QWidget::paintEvent( pe );
	QPainter painter( this );
	painter.setPen( Qt::black );
	painter.drawRect( 5, 5, width() - 10, 10 );
	KisStrategyColorSpaceSP colorSpace = KisColorSpaceRegistry::singleton()->get("RGBA");
	KisPaintDeviceSP device = new KisPaintDevice( width() - 12, 8, colorSpace, " gradient preview " );
	KisGradientPainter gradientPainter( device );
	gradientPainter.setGradient(*m_autogradientResource );
	gradientPainter.paintGradient(QPoint(0, 3), QPoint( device -> width() - 1, 3), KisGradientPainter::GradientShapeLinear, KisGradientPainter::GradientRepeatNone, 0.20, false );
	
	QPixmap pixmap(TILE_WIDTH, device->height());
 	for (int x = 0; x < device -> width(); x += TILE_WIDTH) {
		QImage img = device -> convertToImage(x, 0, TILE_WIDTH, device->height() );
		if (!img.isNull()) {
			m_pixmapIO.putImage(&pixmap, 0, 0, &img);
			painter.drawPixmap(x + 6, 6, pixmap, 0, 0, pixmap.width(), pixmap.height());
		}
 	}
}

void KisGradientSliderWidget::mouseReleaseEvent ( QMouseEvent * e )
{
	QWidget::mouseReleaseEvent( e );
	if( ( e->y() < 5 || e->y() > 15 ) || ( e->x() < 5 || e->x() > width() - 5 ) )
		return;
	double t = (e->x() - 5.0) / (width() - 10.0);
	kdDebug() << "clicked y=" << e->y() << " x=" << e->x() << " t=" << t << endl;
}
