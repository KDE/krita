/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoInteractionTool.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>

#include "KoGfxEvent.h"
#include "KoShape.h"
#include "KoSelection.h"
#include "KoShapeManager.h"
#include "KoInteractionStrategy.h"
#include "KoCanvasBase.h"

#include <kcommand.h>

#define HANDLE_DISTANCE 10

QPointF KoInteractionTool::m_handleDiff[] = {
    QPointF( 0, -HANDLE_DISTANCE ),
    QPointF( HANDLE_DISTANCE, -HANDLE_DISTANCE ),
    QPointF( HANDLE_DISTANCE, 0 ),
    QPointF( HANDLE_DISTANCE, HANDLE_DISTANCE ),
    QPointF( 0, HANDLE_DISTANCE ),
    QPointF( -HANDLE_DISTANCE, HANDLE_DISTANCE ),
    QPointF( -HANDLE_DISTANCE, 0 ),
    QPointF( -HANDLE_DISTANCE, -HANDLE_DISTANCE )
};

KoInteractionTool::KoInteractionTool( const QString & name, const QString & id, const QString & type, KoCanvasBase *canvas )
: KoTool( name, id, type, canvas )
, m_currentStrategy( 0 )
{
}

KoInteractionTool::KoInteractionTool( KoCanvasBase *canvas )
: KoTool( "", "", "", canvas )
, m_currentStrategy( 0 )
{
}

KoInteractionTool::~KoInteractionTool()
{
    delete m_currentStrategy;
}

bool KoInteractionTool::wantsAutoScroll()
{
    return true;
}

QCursor KoInteractionTool::cursor( const QPointF &position )
{
    // TODO maybe introduce concept of MouseMeaning?
    KoShape * shape( m_canvas->shapeManager()->getObjectAt( position ) );
    if ( shape && m_canvas->shapeManager()->selection()->isSelected( shape ) )
    {
        return Qt::SizeAllCursor;
    }
    return Qt::ArrowCursor;
}

void KoInteractionTool::paint( QPainter &painter, KoViewConverter &converter) {
    if ( m_currentStrategy )
        m_currentStrategy->paint( painter, converter);
    else if(m_drawHandles) {
        painter.save();
        painter.setRenderHint( QPainter::Antialiasing, false );
        QPen pen( Qt::blue ); //TODO make it configurable
        painter.setPen( pen );
        bool editable=false;
        foreach(KoShape *shape, selection()->selectedObjects(KoFlake::StrippedSelection)) {
            painter.drawRect( converter.normalToView(shape->boundingRect()) );
            if(!shape->isLocked())
                editable = true;
        }
        painter.restore();
        if( !editable)
            return;

        SelectionDecorator decorator(selection()->boundingRect(), m_lastHandle, true, true);
        decorator.paint(painter, converter);
    }
}

void KoInteractionTool::mousePressEvent( KoGfxEvent *event ) {
    Q_ASSERT(m_currentStrategy == 0);
    m_currentStrategy = KoInteractionStrategy::createStrategy(event, this, m_canvas);
}

void KoInteractionTool::mouseMoveEvent( KoGfxEvent *event ) {
    if(m_currentStrategy) {
        m_lastPoint = event->point;
        m_currentStrategy->handleMouseMove( m_lastPoint, event->modifiers() );
    }
    else {
        if(selection()->count() > 0) {
            m_drawHandles = true;
            QRectF bound = handlesSize();
            if(bound.contains(event->point)) {
                KoFlake::SelectionHandle newDirection = handleAt(event->point);
                if(m_lastHandle != newDirection) {
                    m_lastHandle = newDirection;
                    repaintDecorations();
                }
            }
        } else
            m_drawHandles = false;
        event->ignore();
    }
}

QRectF KoInteractionTool::handlesSize() {
    QRectF bound = selection()->boundingRect();
    // expansion Border
    QPointF border = m_canvas->viewConverter()->viewToNormal(QPointF(HANDLE_DISTANCE, HANDLE_DISTANCE));
    bound.adjust(-border.x(), -border.y(), border.x(), border.y());
    return bound;
}

void KoInteractionTool::mouseReleaseEvent( KoGfxEvent *event ) {
    Q_UNUSED(event);
    if ( m_currentStrategy )
    {
        m_currentStrategy->finishInteraction();
        KCommand *command = m_currentStrategy->createCommand();
        if(command)
            m_canvas->addCommand(command, false);
        delete m_currentStrategy;
        m_currentStrategy = 0;
        repaintDecorations();
    }
    else
        event->ignore();
}

void KoInteractionTool::keyPressEvent(QKeyEvent *event) {
    if(m_currentStrategy && 
       (event->key() == Qt::Key_Control ||
            event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
            event->key() == Qt::Key_Meta)) {
        m_currentStrategy->handleMouseMove( m_lastPoint, event->modifiers() );
    }
}

void KoInteractionTool::keyReleaseEvent(QKeyEvent *event) {
    if(m_currentStrategy == 0)
        ; // catch all cases where no current strategy is needed
    else if(event->key() == Qt::Key_Escape) {
        m_currentStrategy->cancelInteraction();
        delete m_currentStrategy;
        m_currentStrategy = 0;
        event->accept();
    }
    else if(event->key() == Qt::Key_Control ||
            event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
            event->key() == Qt::Key_Meta) {
        m_currentStrategy->handleMouseMove( m_lastPoint, event->modifiers() );
    }
}

void KoInteractionTool::repaintDecorations() {
    if ( selection()->count() > 0 )
    {
        m_canvas->updateCanvas(handlesSize());
    }
}

KoSelection *KoInteractionTool::selection() {
    return m_canvas->shapeManager()->selection();
}

KoFlake::SelectionHandle KoInteractionTool::handleAt(const QPointF &point, bool *innerHandleMeaning) {
    recalcSelectionBox();
    KoViewConverter *converter = m_canvas->viewConverter();

    if(innerHandleMeaning != 0)
        *innerHandleMeaning = false;
    for ( int i = 0; i < KoFlake::NoHandle; ++i ) {
        QPointF pt = converter->normalToView( point ) - converter->normalToView( m_selectionBox[i] ) - m_handleDiff[i];
        // if between outline and out handles;
        if(qAbs(pt.x()) < HANDLE_DISTANCE && qAbs(pt.y()) < HANDLE_DISTANCE)
            return static_cast<KoFlake::SelectionHandle> (i);

        // if just inside the outline
        pt = pt + m_handleDiff[i];
        if(innerHandleMeaning != 0 && qAbs(pt.x()) < HANDLE_DISTANCE &&
                qAbs(pt.y()) < HANDLE_DISTANCE) {
            *innerHandleMeaning = true;
            return static_cast<KoFlake::SelectionHandle> (i);
        }
    }
    return KoFlake::NoHandle;
}

void KoInteractionTool::recalcSelectionBox() {
    QRectF bb( selection()->boundingRect() );
    float width = bb.width();
    float height = bb.height();
    float halfWidth = width / 2.0;
    float halfHeight = height / 2.0;
    float xOff = bb.topLeft().x();
    float yOff = bb.topLeft().y();
    m_selectionBox[KoFlake::TopMiddleHandle] = QPointF( halfWidth + xOff, yOff );
    m_selectionBox[KoFlake::TopRightHandle] = QPointF( width + xOff, yOff );
    m_selectionBox[KoFlake::RightMiddleHandle] = QPointF( width + xOff, halfHeight + yOff );
    m_selectionBox[KoFlake::BottomRightHandle] = QPointF( width + xOff, height + yOff );
    m_selectionBox[KoFlake::BottomMiddleHandle] = QPointF( halfWidth + xOff, height + yOff );
    m_selectionBox[KoFlake::BottomLeftHandle] = QPointF( xOff, height + yOff );
    m_selectionBox[KoFlake::LeftMiddleHandle] = QPointF( xOff, halfHeight + yOff );
    m_selectionBox[KoFlake::TopLeftHandle] = QPointF( xOff, yOff );
}


// ##########  SelectionDecorator ############

SelectionDecorator::SelectionDecorator(QRectF bounds, KoFlake::SelectionHandle arrows,
        bool rotationHandles, bool shearHandles)
: m_rotationHandles(rotationHandles)
, m_shearHandles(shearHandles)
, m_arrows(arrows)
, m_bounds(bounds)
{
}

void SelectionDecorator::paint(QPainter &painter, KoViewConverter &converter) {
    QPen pen( Qt::red ); //TODO make it configurable
    pen.setWidthF(1.2);
    painter.setPen(pen);

    QRectF bounds = converter.normalToView(m_bounds);

    // the 4 move rects
    pen.setWidthF(0);
    painter.setPen(pen);
    QRectF rect(bounds.topLeft(), QSizeF(6, 6));
    painter.drawRect(rect);
    rect.moveLeft(bounds.x() + bounds.width() /2 -3);
    painter.drawRect(rect);
    rect.moveBottom(bounds.bottom());
    painter.drawRect(rect);
    rect.moveLeft(bounds.left());
    painter.drawRect(rect);
    rect.moveTop(bounds.top() + bounds.height() / 2 -3);
    painter.drawRect(rect);
    rect.moveRight(bounds.right());
    painter.drawRect(rect);
    rect.moveBottom(bounds.bottom());
    painter.drawRect(rect);
    rect.moveTop(bounds.top());
    painter.drawRect(rect);

    QPointF border(HANDLE_DISTANCE, HANDLE_DISTANCE);
    bounds.adjust(-border.x(), -border.y(), border.x(), border.y());

    if(m_rotationHandles) {
        QRectF rect(bounds.topLeft(), QSizeF(10, 10));
        painter.drawArc(rect, 16 * 100, 16 * 70);
        rect.moveRight(bounds.right());
        painter.drawArc(rect, 16 * 370, 16 * 70);
        rect.moveBottom(bounds.bottom());
        painter.drawArc(rect, 16 * 280, 16 * 70);
        rect.moveLeft(bounds.left());
        painter.drawArc(rect, 16 * 190, 16 * 70);
    }

    /*if(m_shearHandles) {
        pen.setWidthF(0);
        painter.setPen(pen);
        QRectF rect(bounds.topLeft(), QSizeF(6, 6));
        rect.moveLeft(bounds.x() + bounds.width() /2 -3);
        painter.drawRect(rect);
        rect.moveBottom(bounds.bottom());
        painter.drawRect(rect);
        rect.moveLeft(bounds.left());
        rect.moveTop(bounds.top() + bounds.width() / 2 -3);
        painter.drawRect(rect);
        rect.moveRight(bounds.right());
        painter.drawRect(rect);
    } */
}
