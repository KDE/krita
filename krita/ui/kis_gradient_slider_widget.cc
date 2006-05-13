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

#include <QPainter>
#include <QContextMenuEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QPolygon>
#include <QPaintEvent>

#include <kdebug.h>
#include <kmenu.h>
#include <klocale.h>
#include <kaction.h>

#include "kis_autogradient_resource.h"

#define MARGIN 5
#define HANDLE_SIZE 10

KisGradientSliderWidget::KisGradientSliderWidget(QWidget *parent, const char* name, Qt::WFlags f )
    : QWidget( parent, f),
    m_currentSegment(0),
    m_selectedSegment(0),
    m_drag(0)
{
    setObjectName(name);
    setMinimumHeight(30);

    m_segmentMenu = new KMenu();
    m_segmentMenu->addAction(i18n("Split Segment"), this, SLOT(slotSplitSegment()));
    m_segmentMenu->addAction(i18n("Duplicate Segment"), this, SLOT(slotDuplicateSegment()));
    m_segmentMenu->addAction(i18n("Mirror Segment"), this, SLOT(slotMirrorSegment()));

    m_removeSegmentAction = new KAction(i18n("Remove Segment"), 0, "remove_segment");
    connect(m_removeSegmentAction, SIGNAL(triggered()), this, SLOT(slotRemoveSegment()));

    m_segmentMenu->addAction(m_removeSegmentAction);
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
    QPainter painter(this);
    painter.fillRect(rect(), palette ().background());
    painter.setPen( Qt::black );
    painter.drawRect( MARGIN, MARGIN, width() - 2 * MARGIN, height()- 2 * MARGIN - HANDLE_SIZE );
    if(m_autogradientResource)
    {
        QImage img = m_autogradientResource->generatePreview(width()- 2* MARGIN - 2, height()- 2* MARGIN - HANDLE_SIZE - 2);
        QPixmap pixmap(img.width(), img.height());
        if (!img.isNull()) {
            painter.drawImage( MARGIN + 1, MARGIN + 1, img);
        }

        painter.fillRect( MARGIN + 1, height()- MARGIN - HANDLE_SIZE, width() - 2 * MARGIN, HANDLE_SIZE, QBrush( Qt::white ) );
        if( m_selectedSegment )
        {
            QRect selection( qRound( m_selectedSegment->startOffset()*(double)(width()- 2 * MARGIN - 2) ) + 6,
                    height()- HANDLE_SIZE - MARGIN,
                    qRound( ( m_selectedSegment->endOffset() - m_selectedSegment->startOffset() )*(double)(width()-12) ),
                    HANDLE_SIZE );
            painter.fillRect( selection, QBrush( palette().highlight() ) );
        }

        QPolygon triangle(3);
        QList<double> handlePositions = m_autogradientResource->getHandlePositions();
        int position;
        painter.setBrush( QBrush( Qt::black) );
        for (int i = 0; i < handlePositions.count(); i++)
        {
            position = qRound( handlePositions[i] * (double)( width()-12) ) + 6;
            triangle[0] = QPoint(position, height() - HANDLE_SIZE - MARGIN );
            triangle[1] = QPoint(position + (HANDLE_SIZE / 2 - 1), height() - MARGIN );
            triangle[2] = QPoint(position - (HANDLE_SIZE / 2 - 1), height() - MARGIN );
            painter.drawPolygon(triangle);
        }
        painter.setBrush( QBrush( Qt::white ) );
        QList<double> middleHandlePositions = m_autogradientResource->getMiddleHandlePositions();
        for (int i = 0; i < middleHandlePositions.count(); i++)
        {
            position = qRound( middleHandlePositions[i] * (double)(width()-12) ) + 6;
            triangle[0] = QPoint(position, height()-HANDLE_SIZE - MARGIN);
            triangle[1] = QPoint(position + (HANDLE_SIZE / 2 - 2), height() - MARGIN);
            triangle[2] = QPoint(position - (HANDLE_SIZE / 2 - 2), height() - MARGIN);
            painter.drawPolygon(triangle);
        }
    }
}

void KisGradientSliderWidget::mousePressEvent( QMouseEvent * e )
{
    QWidget::mousePressEvent( e );
    if( ( e->y() < MARGIN || e->y() > height() - MARGIN ) || ( e->x() < MARGIN || e->x() > width() - MARGIN ) || e-> button() != Qt::LeftButton )
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
    repaint();
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

    repaint();
}

void KisGradientSliderWidget::contextMenuEvent( QContextMenuEvent * e )
{
    m_removeSegmentAction->setEnabled(m_autogradientResource->removeSegmentPossible());
    m_segmentMenu->popup(e->globalPos());
}

void KisGradientSliderWidget::slotSplitSegment()
{
    m_autogradientResource->splitSegment( m_selectedSegment );
    emit sigSelectedSegment( m_selectedSegment );
    repaint();
}

void KisGradientSliderWidget::slotDuplicateSegment()
{
    m_autogradientResource->duplicateSegment( m_selectedSegment );
    emit sigSelectedSegment( m_selectedSegment );
    repaint();
}

void KisGradientSliderWidget::slotMirrorSegment()
{
    m_autogradientResource->mirrorSegment( m_selectedSegment );
    emit sigSelectedSegment( m_selectedSegment );
    repaint();
}

void KisGradientSliderWidget::slotRemoveSegment()
{
    m_selectedSegment = m_autogradientResource->removeSegment( m_selectedSegment );
    emit sigSelectedSegment( m_selectedSegment );
    repaint();
}

#include "kis_gradient_slider_widget.moc"
