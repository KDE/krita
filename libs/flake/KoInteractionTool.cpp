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

KoInteractionTool::KoInteractionTool( KoCanvasBase *canvas )
: KoTool( canvas )
, m_currentStrategy( 0 )
, m_mouseWasInsideHandles( false )
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
    Q_UNUSED(position); // we assume the mouseMoveEvent has been called.
    if(selection()->count() > 0) { // has a selection
        if(!m_mouseWasInsideHandles) {
            if(m_lastHandle == KoFlake::NoHandle)
                return Qt::ArrowCursor;
            return Qt::IBeamCursor; // TODO make rotation cursor
        }
        switch(m_lastHandle) {
            case KoFlake::BottomMiddleHandle:
            case KoFlake::TopMiddleHandle:
                return Qt::SizeVerCursor;
            case KoFlake::RightMiddleHandle:
            case KoFlake::LeftMiddleHandle:
                return Qt::SizeHorCursor;
            case KoFlake::TopRightHandle:
            case KoFlake::BottomLeftHandle:
                return Qt::SizeHorCursor; // TODO diagonal cursor
            case KoFlake::BottomRightHandle:
            case KoFlake::TopLeftHandle:
                return Qt::SizeHorCursor; // TODO diagonal cursor
            case KoFlake::NoHandle:
                return Qt::SizeAllCursor;
        }
    }
    return Qt::ArrowCursor;
}

void KoInteractionTool::paint( QPainter &painter, KoViewConverter &converter) {
    if ( m_currentStrategy )
        m_currentStrategy->paint( painter, converter);
    else if(selection()->count() > 0) {
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

        SelectionDecorator decorator(selection()->boundingRect(),
                m_mouseWasInsideHandles?m_lastHandle:KoFlake::NoHandle, true, true);
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
            QRectF bound = handlesSize();
            if(bound.contains(event->point)) {
                bool inside;
                KoFlake::SelectionHandle newDirection = handleAt(event->point, &inside);
                if(inside != m_mouseWasInsideHandles || m_lastHandle != newDirection) {
                    m_lastHandle = newDirection;
                    m_mouseWasInsideHandles = inside;
                    repaintDecorations();
                }
            } else {
                if(m_lastHandle != KoFlake::NoHandle)
                    repaintDecorations();
                m_lastHandle = KoFlake::NoHandle;
                m_mouseWasInsideHandles = false;
            }
        }
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
        m_canvas->updateCanvas(handlesSize());
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
QImage * SelectionDecorator::s_rotateCursor=0;

SelectionDecorator::SelectionDecorator(const QRectF &bounds, KoFlake::SelectionHandle arrows,
        bool rotationHandles, bool shearHandles)
: m_rotationHandles(rotationHandles)
, m_shearHandles(shearHandles)
, m_arrows(arrows)
, m_bounds(bounds)
{
    if(SelectionDecorator::s_rotateCursor == 0) {
        s_rotateCursor = new QImage();
        s_rotateCursor->load("/home/zander/sources/kde4/flake/lib/rotate.png");
        //sd.setObject(&SelectionDecorator::s_rotateCursor, SelectionDecorator::s_rotateCursor);
    }
}

void SelectionDecorator::paint(QPainter &painter, KoViewConverter &converter) {
    QPen pen( Qt::black );
    pen.setWidthF(1.2);
    painter.setPen(pen);
    painter.setBrush(Qt::yellow);

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

    // draw the move arrow(s)
    if(m_arrows != KoFlake::NoHandle && bounds.width() > 45 && bounds.height() > 45) {
        double x1,x2,y1,y2; // 2 is where the arrow head is
        switch(m_arrows) {
            case KoFlake::TopMiddleHandle:
                x1=bounds.center().x(); x2=x1; y2=bounds.y()+8; y1=y2+20;
                break;
            case KoFlake::TopRightHandle:
                x2=bounds.right()-8; x1=x2-20; y2=bounds.y()+8; y1=y2+20;
                break;
            case KoFlake::RightMiddleHandle:
                x2=bounds.right()-8; x1=x2-20; y1=bounds.center().y(); y2=y1;
                break;
            case KoFlake::BottomRightHandle:
                x2=bounds.right()-8; x1=x2-20; y2=bounds.bottom()-8; y1=y2-20;
                break;
            case KoFlake::BottomMiddleHandle:
                x1=bounds.center().x(); x2=x1; y2=bounds.bottom()-8; y1=y2-20;
                break;
            case KoFlake::BottomLeftHandle:
                x2=bounds.left()+8; x1=x2+20; y2=bounds.bottom()-8; y1=y2-20;
                break;
            case KoFlake::LeftMiddleHandle:
                x2=bounds.left()+8; x1=x2+20; y1=bounds.center().y(); y2=y1;
                break;
            default:
            case KoFlake::TopLeftHandle:
                x2=bounds.left()+8; x1=x2+20; y2=bounds.y()+8; y1=y2+20;
                break;
        }
        painter.drawLine(QLineF(x1, y1, x2, y2));
        //pen.setColor(Qt::white);
        //painter.setPen(pen);
        //painter.drawLine(QLineF(x1-1, y1-1, x2-1, y2-1));
    }

    QPointF border(HANDLE_DISTANCE, HANDLE_DISTANCE);
    bounds.adjust(-border.x(), -border.y(), border.x(), border.y());

    if(m_rotationHandles) {
        painter.save();
        painter.translate(bounds.x(), bounds.y());
        QRectF rect(QPointF(0,0), QSizeF(22, 22));
        painter.drawImage(rect, *s_rotateCursor, rect);
        painter.translate(bounds.width(), 0);
        painter.rotate(90);
        if(bounds.width() > 45 && bounds.height() > 45)
            painter.drawImage(rect, *s_rotateCursor, rect);
        painter.translate(bounds.height(), 0);
        painter.rotate(90);
        painter.drawImage(rect, *s_rotateCursor, rect);
        painter.translate(bounds.width(), 0);
        painter.rotate(90);
        if(bounds.width() > 45 && bounds.height() > 45)
            painter.drawImage(rect, *s_rotateCursor, rect);
        painter.restore();
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
