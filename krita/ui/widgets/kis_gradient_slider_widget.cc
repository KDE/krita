/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <longamp@reallygood.de>
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

KisGradientSliderWidget::KisGradientSliderWidget(QWidget *parent, const char* name, WFlags f )
	: QWidget( parent, name, f), 
	m_currentSegment(0)
{
	setMinimumHeight(30);
}

void KisGradientSliderWidget::paintEvent ( QPaintEvent* pe )
{
	QWidget::paintEvent( pe );
	QPainter painter( this );
	painter.setPen( Qt::black );
	painter.drawRect( 5, 5, width() - 10, height()-15 );
	QImage img = m_autogradientResource -> generatePreview(width()-12, height()-17);
	QPixmap pixmap(img.width(), img.height());
	if (!img.isNull()) {
		m_pixmapIO.putImage(&pixmap, 0, 0, &img);
		painter.drawPixmap(6, 6, pixmap, 0, 0, pixmap.width(), pixmap.height());
	}
}

void KisGradientSliderWidget::mouseReleaseEvent ( QMouseEvent * e )
{
	QWidget::mouseReleaseEvent( e );
	if( ( e->y() < 5 || e->y() > 15 ) || ( e->x() < 5 || e->x() > width() - 5 ) )
		return;
	double t = (e->x() - 5.0) / (width() - 10.0);
	kdDebug() << "clicked y=" << e->y() << " x=" << e->x() << " t=" << t << endl;
	KisGradientSegment* segment = m_autogradientResource -> segmentAt(t);
	if(segment != 0)
		m_currentSegment = segment;
		emit sigSelectedSegment(segment);
}

#include "kis_gradient_slider_widget.moc"
