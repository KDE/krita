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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_gradient_slider_widget.h"

#include <qpainter.h>

#include <kdebug.h>
#include <kpopupmenu.h>
#include <klocale.h>

#include "kis_autogradient_resource.h"

#define MARGIN 5
#define HANDLE_SIZE 10

KisGradientSliderWidget::KisGradientSliderWidget(QWidget *parent, const char* name, WFlags f )
    : QWidget( parent, name, f),
    m_currentSegment(0),
    m_selectedSegment(0),
    m_drag(0)
{
    setMinimumHeight(30);

    m_segmentMenu = new KPopupMenu();
    m_segmentMenu->insertItem(i18n("Split Segment"), SPLIT_SEGMENT);
    m_segmentMenu->insertItem(i18n("Duplicate Segment"), DUPLICATE_SEGMENT);
    m_segmentMenu->insertItem(i18n("Mirror Segment"), MIRROR_SEGMENT);
    m_segmentMenu->insertItem(i18n("Remove Segment"), REMOVE_SEGMENT);
    connect( m_segmentMenu, SIGNAL( activated(int) ), SLOT( slotMenuAction(int) ) );
}

void KisGradientSliderWidget::setGradientResource( KisAutogradientResource* agr)
{
    m_autogradientResource = agr;
    m_selectedSegment = m_autogradientResource->segmentAt(0.0);
    emit sigSelectedSegment( m_selectedSegment );
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
        QImage img = m_autogradientResource->generatePreview(width()- 2* MARGIN - 2, height()- 2* MARGIN - HANDLE_SIZE - 2);
        QPixmap pixmap(img.width(), img.height());
        if (!img.isNull()) {
            m_pixmapIO.putImage(&pixmap, 0, 0, &img);
            painter.drawPixmap( MARGIN + 1, MARGIN + 1, pixmap, 0, 0, pixmap.width(), pixmap.height());
        }

        painter.fillRect( MARGIN + 1, height()- MARGIN - HANDLE_SIZE, width() - 2 * MARGIN, HANDLE_SIZE, QBrush( Qt::white ) );
        if( m_selectedSegment )
        {
            QRect selection( qRound( m_selectedSegment->startOffset()*(double)(width()- 2 * MARGIN - 2) ) + 6,
                    height()- HANDLE_SIZE - MARGIN,
                    qRound( ( m_selectedSegment->endOffset() - m_selectedSegment->startOffset() )*(double)(width()-12) ),
                    HANDLE_SIZE );
            painter.fillRect( selection, QBrush( colorGroup().highlight() ) );
        }

        QPointArray triangle(3);
        QValueVector<double> handlePositions = m_autogradientResource->getHandlePositions();
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
        painter.setBrush( QBrush( Qt::white ) );
        QValueVector<double> middleHandlePositions = m_autogradientResource->getMiddleHandlePositions();
        for (uint i = 0; i < middleHandlePositions.count(); i++)
        {
            position = qRound( middleHandlePositions[i] * (double)(width()-12) ) + 6;
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
    QWidget::mousePressEvent( e );
    if( ( e->y() < MARGIN || e->y() > height() - MARGIN ) || ( e->x() < MARGIN || e->x() > width() - MARGIN ) || e-> button() != LeftButton )
        return;
    double t = (double)(e->x() - MARGIN) / (double)(width() - 2 * MARGIN);
    KisGradientSegment* segment = 0;
    segment = m_autogradientResource->segmentAt(t);
    if(segment != 0)
    {
        m_currentSegment = segment;
        QRect leftHandle( qRound(m_currentSegment->startOffset() * (double)(width()-2*MARGIN-2)+ MARGIN - (HANDLE_SIZE/2 - 1 )),
                    height() - HANDLE_SIZE,
                    HANDLE_SIZE - 1,
                    HANDLE_SIZE);
        QRect middleHandle( qRound(m_currentSegment->middleOffset() * (double)(width()-2*MARGIN-2)+ MARGIN - (HANDLE_SIZE/2 -2) ),
                    height() - HANDLE_SIZE - MARGIN,
                    HANDLE_SIZE - 1,
                    HANDLE_SIZE);
        QRect rightHandle( qRound(m_currentSegment->endOffset() * (double)(width()-2*MARGIN-2)+ MARGIN - (HANDLE_SIZE/2 - 1 )),
                    height() - HANDLE_SIZE,
                    HANDLE_SIZE - 1,
                    HANDLE_SIZE);
        // Change the activation order of the handles to avoid deadlocks
        if( t > 0.5 )
        {
            if( leftHandle.contains( e->pos() ) )
                m_drag = LEFT_DRAG;
            else if( middleHandle.contains( e->pos() ) )
                m_drag = MIDDLE_DRAG;
            else if( rightHandle.contains( e->pos() ) )
                m_drag = RIGHT_DRAG;
        }
        else
        {
            if( rightHandle.contains( e->pos() ) )
                m_drag = RIGHT_DRAG;
            else if( middleHandle.contains( e->pos() ) )
                m_drag = MIDDLE_DRAG;
            else if( leftHandle.contains( e->pos() ) )
                m_drag = LEFT_DRAG;
        }

        if( m_drag == NO_DRAG )
        {
            m_selectedSegment = m_currentSegment;
            emit sigSelectedSegment( m_selectedSegment );
        }
    }
    repaint(false);
}

void KisGradientSliderWidget::mouseReleaseEvent ( QMouseEvent * e )
{
    QWidget::mouseReleaseEvent( e );
    m_drag = NO_DRAG;
}

void KisGradientSliderWidget::mouseMoveEvent( QMouseEvent * e )
{
    QWidget::mouseMoveEvent( e );
    if( ( e->y() < MARGIN || e->y() > height() - MARGIN ) || ( e->x() < MARGIN || e->x() > width() - MARGIN ) )
        return;
    double t = (double)(e->x() - MARGIN) / (double)(width() - 2 * MARGIN);
    switch( m_drag )
    {
        case RIGHT_DRAG:
            m_autogradientResource->moveSegmentEndOffset( m_currentSegment, t );
            break;
        case LEFT_DRAG:
            m_autogradientResource->moveSegmentStartOffset( m_currentSegment, t );
            break;
        case MIDDLE_DRAG:
            m_autogradientResource->moveSegmentMiddleOffset( m_currentSegment, t );
            break;
    }

    if ( m_drag != NO_DRAG)
        emit sigChangedSegment( m_currentSegment );

    repaint(false);
}

void KisGradientSliderWidget::contextMenuEvent( QContextMenuEvent * e )
{
    m_segmentMenu->setItemEnabled( REMOVE_SEGMENT, m_autogradientResource->removeSegmentPossible() );
    m_segmentMenu->popup( e->globalPos());
}

void KisGradientSliderWidget::slotMenuAction( int id )
{
    switch( id )
    {
        case SPLIT_SEGMENT:
            m_autogradientResource->splitSegment( m_selectedSegment );
            break;
        case DUPLICATE_SEGMENT:
            m_autogradientResource->duplicateSegment( m_selectedSegment );
            break;
        case MIRROR_SEGMENT:
            m_autogradientResource->mirrorSegment( m_selectedSegment );
            break;
        case REMOVE_SEGMENT:
            m_selectedSegment = m_autogradientResource->removeSegment( m_selectedSegment );
            break;
    }
    emit sigSelectedSegment( m_selectedSegment );
    repaint(false);
}

#include "kis_gradient_slider_widget.moc"
