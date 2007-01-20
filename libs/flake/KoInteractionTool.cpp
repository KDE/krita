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

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QBitmap>

#include "KoPointerEvent.h"
#include "KoShape.h"
#include "KoSelection.h"
#include "KoShapeManager.h"
#include "KoInteractionStrategy.h"
#include "KoCanvasBase.h"
#include "commands/KoShapeMoveCommand.h"

#include <QUndoCommand>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kdebug.h>

#define HANDLE_DISTANCE 10

KoInteractionTool::KoInteractionTool( KoCanvasBase *canvas )
: KoTool( canvas )
, m_currentStrategy( 0 )
, m_lastHandle(KoFlake::NoHandle)
, m_mouseWasInsideHandles( false )
, m_moveCommand(0)
{
    QPixmap rotatePixmap, shearPixmap;
    rotatePixmap.load(KStandardDirs::locate("data", "koffice/icons/rotate.png"));
    shearPixmap.load(KStandardDirs::locate("data", "koffice/icons/shear.png"));

    m_rotateCursors[0] = QCursor(rotatePixmap.transformed(QMatrix().rotate(45)));
    m_rotateCursors[1] = QCursor(rotatePixmap.transformed(QMatrix().rotate(90)));
    m_rotateCursors[2] = QCursor(rotatePixmap.transformed(QMatrix().rotate(135)));
    m_rotateCursors[3] = QCursor(rotatePixmap.transformed(QMatrix().rotate(180)));
    m_rotateCursors[4] = QCursor(rotatePixmap.transformed(QMatrix().rotate(225)));
    m_rotateCursors[5] = QCursor(rotatePixmap.transformed(QMatrix().rotate(270)));
    m_rotateCursors[6] = QCursor(rotatePixmap.transformed(QMatrix().rotate(315)));
    m_rotateCursors[7] = QCursor(rotatePixmap);
/*
    m_rotateCursors[0] = QCursor(Qt::RotateNCursor);
    m_rotateCursors[1] = QCursor(Qt::RotateNECursor);
    m_rotateCursors[2] = QCursor(Qt::RotateECursor);
    m_rotateCursors[3] = QCursor(Qt::RotateSECursor);
    m_rotateCursors[4] = QCursor(Qt::RotateSCursor);
    m_rotateCursors[5] = QCursor(Qt::RotateSWCursor);
    m_rotateCursors[6] = QCursor(Qt::RotateWCursor);
    m_rotateCursors[7] = QCursor(Qt::RotateNWCursor);
*/
    m_shearCursors[0] = QCursor(shearPixmap);
    m_shearCursors[1] = QCursor(shearPixmap.transformed(QMatrix().rotate(45)));
    m_shearCursors[2] = QCursor(shearPixmap.transformed(QMatrix().rotate(90)));
    m_shearCursors[3] = QCursor(shearPixmap.transformed(QMatrix().rotate(135)));
    m_shearCursors[4] = QCursor(shearPixmap.transformed(QMatrix().rotate(180)));
    m_shearCursors[5] = QCursor(shearPixmap.transformed(QMatrix().rotate(225)));
    m_shearCursors[6] = QCursor(shearPixmap.transformed(QMatrix().rotate(270)));
    m_shearCursors[7] = QCursor(shearPixmap.transformed(QMatrix().rotate(315)));
    m_sizeCursors[0] = KCursor::sizeVerCursor();
    m_sizeCursors[1] = KCursor::sizeBDiagCursor();
    m_sizeCursors[2] = KCursor::sizeHorCursor();
    m_sizeCursors[3] = KCursor::sizeFDiagCursor();
    m_sizeCursors[4] = KCursor::sizeVerCursor();
    m_sizeCursors[5] = KCursor::sizeBDiagCursor();
    m_sizeCursors[6] = KCursor::sizeHorCursor();
    m_sizeCursors[7] = KCursor::sizeFDiagCursor();
}

KoInteractionTool::~KoInteractionTool()
{
    delete m_currentStrategy;
    m_moveCommand = 0;
}

bool KoInteractionTool::wantsAutoScroll()
{
    return true;
}

void KoInteractionTool::updateCursor() {
    QCursor cursor = Qt::ArrowCursor;

    if(koSelection()->count() > 0) { // has a selection
        bool editable=false;
        foreach(KoShape *shape, koSelection()->selectedShapes(KoFlake::StrippedSelection)) {
            if(!shape->isLocked())
                editable = true;
        }

        if(koSelection()->count()>1)
            m_angle = koSelection()->rotation();
        else
            m_angle = koSelection()->firstSelectedShape()->rotation();

        int rotOctant = 8 + int(8.5 + m_angle / 45);

        if(!m_mouseWasInsideHandles) {
            switch(m_lastHandle) {
                case KoFlake::TopMiddleHandle:
                    cursor = m_shearCursors[(0 +rotOctant)%8];
                    break;
                case KoFlake::TopRightHandle:
                    cursor = m_rotateCursors[(1 +rotOctant)%8];
                    break;
                case KoFlake::RightMiddleHandle:
                    cursor = m_shearCursors[(2 +rotOctant)%8];
                    break;
                case KoFlake::BottomRightHandle:
                    cursor = m_rotateCursors[(3 +rotOctant)%8];
                    break;
                case KoFlake::BottomMiddleHandle:
                    cursor = m_shearCursors[(4 +rotOctant)%8];
                    break;
                case KoFlake::BottomLeftHandle:
                    cursor = m_rotateCursors[(5 +rotOctant)%8];
                    break;
                case KoFlake::LeftMiddleHandle:
                    cursor = m_shearCursors[(6 +rotOctant)%8];
                    break;
                case KoFlake::TopLeftHandle:
                    cursor = m_rotateCursors[(7 +rotOctant)%8];
                    break;
                case KoFlake::NoHandle:
                    cursor = Qt::ArrowCursor;
                    break;
             }
        }
        else {
            switch(m_lastHandle) {
                case KoFlake::TopMiddleHandle:
                    cursor = m_sizeCursors[(0 +rotOctant)%8];
                    break;
                case KoFlake::TopRightHandle:
                    cursor = m_sizeCursors[(1 +rotOctant)%8];
                    break;
                case KoFlake::RightMiddleHandle:
                    cursor = m_sizeCursors[(2 +rotOctant)%8];
                    break;
                case KoFlake::BottomRightHandle:
                    cursor = m_sizeCursors[(3 +rotOctant)%8];
                    break;
                case KoFlake::BottomMiddleHandle:
                    cursor = m_sizeCursors[(4 +rotOctant)%8];
                    break;
                case KoFlake::BottomLeftHandle:
                    cursor = m_sizeCursors[(5 +rotOctant)%8];
                    break;
                case KoFlake::LeftMiddleHandle:
                    cursor = m_sizeCursors[(6 +rotOctant)%8];
                    break;
                case KoFlake::TopLeftHandle:
                    cursor = m_sizeCursors[(7 +rotOctant)%8];
                    break;
                case KoFlake::NoHandle:
                    cursor = Qt::SizeAllCursor;
                    break;
            }
        }
        if( !editable)
            cursor = Qt::ArrowCursor;
    }
    useCursor(cursor);
}

void KoInteractionTool::paint( QPainter &painter, KoViewConverter &converter) {
    if ( m_currentStrategy )
        m_currentStrategy->paint( painter, converter);
    else if(koSelection()->count() > 0) {
        SelectionDecorator decorator(m_mouseWasInsideHandles ? m_lastHandle : KoFlake::NoHandle,
                 true, true);
        decorator.setSelection(koSelection());
        decorator.setHandleRadius( m_canvas->resourceProvider()->handleRadius() );
        decorator.paint(painter, converter);
    }
}

void KoInteractionTool::mousePressEvent( KoPointerEvent *event ) {
    Q_ASSERT(m_currentStrategy == 0);
    m_currentStrategy = KoInteractionStrategy::createStrategy(event, this, m_canvas);
    m_moveCommand = 0;
    updateCursor();
}

void KoInteractionTool::mouseMoveEvent( KoPointerEvent *event ) {
    if(m_currentStrategy) {
        m_lastPoint = event->point;
        m_currentStrategy->handleMouseMove( m_lastPoint, event->modifiers() );
    }
    else {
        if(koSelection()->count() > 0) {
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
    updateCursor();
}

QRectF KoInteractionTool::handlesSize() {
    QRectF bound = koSelection()->boundingRect();
    // expansion Border
    QPointF border = m_canvas->viewConverter()->viewToDocument(QPointF(HANDLE_DISTANCE, HANDLE_DISTANCE));
    bound.adjust(-border.x(), -border.y(), border.x(), border.y());
    return bound;
}

void KoInteractionTool::mouseReleaseEvent( KoPointerEvent *event ) {
    Q_UNUSED(event);
    if ( m_currentStrategy )
    {
        m_currentStrategy->finishInteraction( event->modifiers() );
        QUndoCommand *command = m_currentStrategy->createCommand();
        if(command)
            m_canvas->addCommand(command);
        delete m_currentStrategy;
        m_currentStrategy = 0;
        repaintDecorations();
    }
    else
        event->ignore();
    updateCursor();
}

void KoInteractionTool::keyPressEvent(QKeyEvent *event) {
    if(m_currentStrategy &&
       (event->key() == Qt::Key_Control ||
            event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
            event->key() == Qt::Key_Meta)) {
        m_currentStrategy->handleMouseMove( m_lastPoint, event->modifiers() );
    } else if(m_currentStrategy == 0) {
        double x=0.0, y=0.0;
        if(event->key() == Qt::Key_Left)
            x=-0.5;
        else if(event->key() == Qt::Key_Right)
            x=0.5;
        else if(event->key() == Qt::Key_Up)
            y=-0.5;
        else if(event->key() == Qt::Key_Down)
            y=0.5;

        if(x != 0.0 || y != 0.0) { // actually move
            if((event->modifiers() & Qt::ShiftModifier) == 0) { // no shift used
                x *= 10;
                y *= 10;
            }

            QList<QPointF> prevPos;
            QList<QPointF> newPos;
            QList<KoShape*> shapes;
            foreach(KoShape* shape, koSelection()->selectedShapes(KoFlake::StrippedSelection)) {
                if(shape->isLocked())
                    continue;
                shapes.append(shape);
                QPointF p = shape->position();
                prevPos.append(p);
                p.setX(p.x() + x);
                p.setY(p.y() + y);
                newPos.append(p);
            }
            if(shapes.count() > 0) {
                // use a timeout to make sure we don't reuse a command possibly deleted by the commandHistory
                if(m_lastUsedMoveCommand.msecsTo(QTime::currentTime()) > 5000)
                    m_moveCommand = 0;
                if(m_moveCommand) { // alter previous instead of creating new one.
                    m_moveCommand->setNewPositions(newPos);
                    m_moveCommand->redo();
                } else {
                    m_moveCommand = new KoShapeMoveCommand(shapes, prevPos, newPos);
                    m_canvas->addCommand(m_moveCommand);
                }
                m_lastUsedMoveCommand = QTime::currentTime();
            }
        }
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
    if ( koSelection()->count() > 0 )
        m_canvas->updateCanvas(handlesSize());
}

KoSelection *KoInteractionTool::koSelection() {
    return m_canvas->shapeManager()->selection();
}

KoFlake::SelectionHandle KoInteractionTool::handleAt(const QPointF &point, bool *innerHandleMeaning) {
    // check for handles in this order; meaning that when handles overlap the one on top is chosen
    static const KoFlake::SelectionHandle handleOrder[] = {
        KoFlake::BottomRightHandle,
        KoFlake::TopLeftHandle,
        KoFlake::BottomLeftHandle,
        KoFlake::TopRightHandle,
        KoFlake::BottomMiddleHandle,
        KoFlake::RightMiddleHandle,
        KoFlake::LeftMiddleHandle,
        KoFlake::TopMiddleHandle,
        KoFlake::NoHandle
    };

    if( koSelection()->count() == 0 )
        return KoFlake::NoHandle;

    recalcSelectionBox();
    KoViewConverter *converter = m_canvas->viewConverter();

    if(innerHandleMeaning != 0)
    {
        QPainterPath path;
        path.addPolygon(m_selectionOutline);
        *innerHandleMeaning =  path.contains(point);
    }
    for ( int i = 0; i < KoFlake::NoHandle; ++i ) {
        KoFlake::SelectionHandle handle = handleOrder[i];
        QPointF pt = converter->documentToView(point) - converter->documentToView(m_selectionBox[handle]);

        // if just inside the outline
        if(qAbs(pt.x()) < HANDLE_DISTANCE &&
                qAbs(pt.y()) < HANDLE_DISTANCE) {
            if(innerHandleMeaning != 0)
            {
                if(qAbs(pt.x()) < 4 && qAbs(pt.y()) < 4)
                    *innerHandleMeaning = true;
            }
            return handle;
        }
    }
    return KoFlake::NoHandle;
}

void KoInteractionTool::recalcSelectionBox() {
    if(koSelection()->count()==0)
        return;

    if(koSelection()->count()>1)
    {
        QMatrix matrix = koSelection()->transformationMatrix(0);
        m_selectionOutline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), koSelection()->size())));
        m_angle = koSelection()->rotation();
    }
    else
    {
        QMatrix matrix = koSelection()->firstSelectedShape()->transformationMatrix(0);
        m_selectionOutline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), koSelection()->firstSelectedShape()->size())));
        m_angle = koSelection()->firstSelectedShape()->rotation();
    }
    QPolygonF outline = m_selectionOutline; //shorter name in the following :)
    m_selectionBox[KoFlake::TopMiddleHandle] = (outline.value(0)+outline.value(1))/2;
    m_selectionBox[KoFlake::TopRightHandle] = outline.value(1);
    m_selectionBox[KoFlake::RightMiddleHandle] = (outline.value(1)+outline.value(2))/2;
    m_selectionBox[KoFlake::BottomRightHandle] = outline.value(2);
    m_selectionBox[KoFlake::BottomMiddleHandle] = (outline.value(2)+outline.value(3))/2;
    m_selectionBox[KoFlake::BottomLeftHandle] = outline.value(3);
    m_selectionBox[KoFlake::LeftMiddleHandle] = (outline.value(3)+outline.value(0))/2;
    m_selectionBox[KoFlake::TopLeftHandle] = outline.value(0);
    if(koSelection()->count() == 1) {
        KoShape *s = koSelection()->firstSelectedShape();
        if(s->scaleX() < 0) { // vertically mirrored: swap left / right
            qSwap(m_selectionBox[KoFlake::TopLeftHandle], m_selectionBox[KoFlake::TopRightHandle]);
            qSwap(m_selectionBox[KoFlake::LeftMiddleHandle], m_selectionBox[KoFlake::RightMiddleHandle]);
            qSwap(m_selectionBox[KoFlake::BottomLeftHandle], m_selectionBox[KoFlake::BottomRightHandle]);
        }
        if(s->scaleY() < 0) { // vertically mirrored: swap top / bottom
            qSwap(m_selectionBox[KoFlake::TopLeftHandle], m_selectionBox[KoFlake::BottomLeftHandle]);
            qSwap(m_selectionBox[KoFlake::TopMiddleHandle], m_selectionBox[KoFlake::BottomMiddleHandle]);
            qSwap(m_selectionBox[KoFlake::TopRightHandle], m_selectionBox[KoFlake::BottomRightHandle]);
        }
    }
}

void KoInteractionTool::activate(bool temporary) {
    Q_UNUSED(temporary);
    m_mouseWasInsideHandles = false;
    m_lastHandle = KoFlake::NoHandle;
    useCursor(Qt::ArrowCursor, true);
    repaintDecorations();
}

void KoInteractionTool::deactivate() {
    repaintDecorations();
}

// ##########  SelectionDecorator ############
QImage * SelectionDecorator::s_rotateCursor=0;
static KStaticDeleter<QImage> staticRotateCursorDeleter;

SelectionDecorator::SelectionDecorator(KoFlake::SelectionHandle arrows,
        bool rotationHandles, bool shearHandles)
: m_rotationHandles(rotationHandles)
, m_shearHandles(shearHandles)
, m_arrows(arrows)
, m_handleRadius( 3 )
{
    if(SelectionDecorator::s_rotateCursor == 0) {
        staticRotateCursorDeleter.setObject(s_rotateCursor, new QImage());
        s_rotateCursor->load(KStandardDirs::locate("lib", "flake/rotate.png"));
    }
}

void SelectionDecorator::setSelection(KoSelection *selection) {
    m_selection = selection;
}

void SelectionDecorator::setHandleRadius( int radius ) {
    m_handleRadius = radius;
}

void SelectionDecorator::paint(QPainter &painter, KoViewConverter &converter) {
    QPen pen( Qt::green );
    QPolygonF outline;

    painter.save();
    painter.setRenderHint( QPainter::Antialiasing, false );
    painter.setPen( pen );
    bool editable=false;
    foreach(KoShape *shape, m_selection->selectedShapes(KoFlake::StrippedSelection)) {
        QMatrix matrix = shape->transformationMatrix(0);
        outline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), shape->size())));
        for(int i =0; i<outline.count(); i++)
            outline[i] = converter.documentToView(outline.value(i));
        painter.drawPolygon(outline);
        if(!shape->isLocked())
            editable = true;
    }
    painter.restore();

    if(m_selection->count()>1)
    {
        QMatrix matrix = m_selection->transformationMatrix(0);
        outline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), m_selection->size())));
        for(int i =0; i<outline.count(); i++)
            outline[i] = converter.documentToView(outline.value(i));
        pen = QPen( Qt::blue );
        painter.setPen(pen);
        painter.drawPolygon(outline);
    }
    else if( m_selection->firstSelectedShape() )
    {
        QMatrix matrix = m_selection->firstSelectedShape()->transformationMatrix(0);
        outline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), m_selection->firstSelectedShape()->size())));
        for(int i =0; i<outline.count(); i++)
            outline[i] = converter.documentToView(outline.value(i));
    }

    if( !editable)
        return;

    pen = QPen( Qt::black );
    pen.setWidthF(1.2);
    painter.setPen(pen);
    painter.setBrush(Qt::yellow);

    // the 8 move rects
    QRectF rect( QPointF(0,0), QSizeF(2*m_handleRadius,2*m_handleRadius) );
    pen.setWidthF(0);
    painter.setPen(pen);
    rect.moveCenter(outline.value(0));
    painter.drawRect(rect);
    rect.moveCenter(outline.value(1));
    painter.drawRect(rect);
    rect.moveCenter(outline.value(2));
    painter.drawRect(rect);
    rect.moveCenter(outline.value(3));
    painter.drawRect(rect);
    rect.moveCenter((outline.value(0)+outline.value(1))/2);
    painter.drawRect(rect);
    rect.moveCenter((outline.value(1)+outline.value(2))/2);
    painter.drawRect(rect);
    rect.moveCenter((outline.value(2)+outline.value(3))/2);
    painter.drawRect(rect);
    rect.moveCenter((outline.value(3)+outline.value(0))/2);
    painter.drawRect(rect);

#if 0
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
#endif
}
