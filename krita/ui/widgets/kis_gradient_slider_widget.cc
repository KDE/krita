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

#define MARGIN 5
#define HANDLE_SIZE 10

KisGradientSliderWidget::KisGradientSliderWidget(QWidget *parent, const char* name, WFlags f )
	: QWidget( parent, name, f), 
	m_currentSegment(0),
	m_drag(0)
{
	setMinimumHeight(50);
}

void KisGradientSliderWidget::setGradientResource( KisAutogradientResource* agr)
{
	m_autogradientResource = agr;
	m_currentSegment = m_autogradientResource -> segmentAt(0.0);
	emit sigSelectedSegment(m_currentSegment);
}

void KisGradientSliderWidget::paintEvent ( QPaintEvent* pe )
{
	QWidget::paintEvent( pe );
	QPixmap pixmap( width(), height() );
	pixmap.fill( colorGroup().background() );
	QPainter painter( &pixmap );
	painter.setPen( Qt::black );
	painter.drawRect( MARGIN, MARGIN, width() - 2 * MARGIN, height()- 2 * MARGIN - HANDLE_SIZE );
	if(m_autogradientResource)
	{
		QImage img = m_autogradientResource -> generatePreview(width()- 2* MARGIN - 2, height()- 2* MARGIN - HANDLE_SIZE - 2);
		QPixmap pixmap(img.width(), img.height());
		if (!img.isNull()) {
			m_pixmapIO.putImage(&pixmap, 0, 0, &img);
			painter.drawPixmap( MARGIN + 1, MARGIN + 1, pixmap, 0, 0, pixmap.width(), pixmap.height());
		}
	
		painter.fillRect( MARGIN + 1, height()- MARGIN - HANDLE_SIZE, width() - 2 * MARGIN, HANDLE_SIZE, QBrush( Qt::white ) );
		QRect selection( qRound( m_currentSegment -> startOffset()*(double)(width()- 2 * MARGIN - 2) ) + 6,
				 height()- HANDLE_SIZE - MARGIN, 
				 qRound( ( m_currentSegment -> endOffset() - m_currentSegment -> startOffset() )*(double)(width()-12) ),
				 HANDLE_SIZE );
		painter.fillRect( selection, QBrush( colorGroup().highlight() ) );
	
		QPointArray triangle(3);
		QValueVector<double> handlePositions = m_autogradientResource -> getHandlePositions();
		int position;
		painter.setBrush( QBrush( Qt::black) );
		for (uint i = 0; i < handlePositions.count(); i++)
		{
			position = qRound( handlePositions[i] * (double)( width()-12) ) + 6;
			triangle[0] = QPoint(position, height() - HANDLE_SIZE - MARGIN );
			triangle[1] = QPoint(position + (HANDLE_SIZE / 2 - 1), height() - MARGIN );
			triangle[2] = QPoint(position - (HANDLE_SIZE / 2 - 1), height() - MARGIN );
			painter.drawPolygon(triangle);
		}
		if(m_currentSegment)
		{
			painter.setBrush( QBrush( Qt::white ) );
			position = qRound(m_currentSegment -> middleOffset() * (double)(width()-12) ) + 6;
			triangle[0] = QPoint(position, height()-HANDLE_SIZE - MARGIN);
			triangle[1] = QPoint(position + (HANDLE_SIZE / 2 - 2), height() - MARGIN);
			triangle[2] = QPoint(position - (HANDLE_SIZE / 2 - 2), height() - MARGIN);
			painter.drawPolygon(triangle);
		}
	}
	bitBlt( this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(), Qt::CopyROP);
}

void KisGradientSliderWidget::mousePressEvent( QMouseEvent * e )
{
	QWidget::mouseReleaseEvent( e );
	if( ( e->y() < MARGIN || e->y() > height() - MARGIN ) || ( e->x() < MARGIN || e->x() > width() - MARGIN ) )
		return;
	double t = (double)(e->x() - MARGIN) / (double)(width() - 2 * MARGIN);
	kdDebug() << "clicked y=" << e->y() << " x=" << e->x() << " t=" << t << endl;
	KisGradientSegment* segment = 0;
	segment = m_autogradientResource -> segmentAt(t);
	if(segment != 0)
	{
		m_currentSegment = segment;
		emit sigSelectedSegment(segment);
		QRect leftHandle( qRound(m_currentSegment -> startOffset() * (double)(width()-2*MARGIN-2)+ MARGIN - (HANDLE_SIZE/2 - 1 )), 
					height() - HANDLE_SIZE,
					HANDLE_SIZE - 1,
					HANDLE_SIZE);
		QRect middleHandle( qRound(m_currentSegment -> middleOffset() * (double)(width()-2*MARGIN-2)+ MARGIN - (HANDLE_SIZE/2 -2) ), 
					height() - HANDLE_SIZE - MARGIN,
					HANDLE_SIZE - 1,
					HANDLE_SIZE);
		QRect rightHandle( qRound(m_currentSegment -> endOffset() * (double)(width()-2*MARGIN-2)+ MARGIN - (HANDLE_SIZE/2 - 1 )), 
					height() - HANDLE_SIZE,
					HANDLE_SIZE - 1,
					HANDLE_SIZE);
		// Change the activation order of the handles to avoid deadlocks
		if( t > 0.5 )
		{
			if( leftHandle.contains( e -> pos() ) )
				m_drag = LEFT_DRAG;
			else if( middleHandle.contains( e -> pos() ) )
				m_drag = MIDDLE_DRAG;
			else if( rightHandle.contains( e -> pos() ) )
				m_drag = RIGHT_DRAG;
		}
		else
		{
			if( rightHandle.contains( e -> pos() ) )
				m_drag = RIGHT_DRAG;
			else if( middleHandle.contains( e -> pos() ) )
				m_drag = MIDDLE_DRAG;
			else if( leftHandle.contains( e -> pos() ) )
				m_drag = LEFT_DRAG;
		}
	}
	repaint(false);
}

void KisGradientSliderWidget::mouseReleaseEvent ( QMouseEvent * e )
{
	m_drag = NO_DRAG;
}

void KisGradientSliderWidget::mouseMoveEvent( QMouseEvent * e )
{
	if( ( e->y() < MARGIN || e->y() > height() - MARGIN ) || ( e->x() < MARGIN || e->x() > width() - MARGIN ) )
		return;
	double t = (double)(e->x() - MARGIN) / (double)(width() - 2 * MARGIN);
	switch( m_drag )
	{
		case RIGHT_DRAG:
			m_autogradientResource -> moveSegmentEndOffset( m_currentSegment, t );
			break;
		case LEFT_DRAG:
			m_autogradientResource -> moveSegmentStartOffset( m_currentSegment, t );
			break;
		case MIDDLE_DRAG:
			m_autogradientResource -> moveSegmentMiddleOffset( m_currentSegment, t );
			break;
	}
	repaint(false);
}

#include "kis_gradient_slider_widget.moc"
