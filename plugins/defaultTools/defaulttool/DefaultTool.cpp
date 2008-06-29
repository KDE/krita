/* This file is part of the KDE project

   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>

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

#include "DefaultTool.h"
#include "DefaultToolWidget.h"
#include "SelectionDecorator.h"
#include "ShapeMoveStrategy.h"
#include "ShapeRotateStrategy.h"
#include "ShapeShearStrategy.h"
#include "ShapeResizeStrategy.h"

#include <KoPointerEvent.h>
#include <KoToolSelection.h>
#include <KoToolManager.h>
#include <KoSelection.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoShapeGroup.h>
#include <KoShapeLayer.h>
#include <KoShapePaste.h>
#include <KoShapeOdfSaveHelper.h>
#include <KoDrag.h>
#include <KoDocument.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeRubberSelectStrategy.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeDeleteCommand.h>
#include <KoSnapGuide.h>

#include <QAction>
#include <QKeyEvent>
#include <QClipboard>
#include <kstandarddirs.h>

#include <math.h>

#define HANDLE_DISTANCE 10

class SelectionHandler : public KoToolSelection
{
public:
    SelectionHandler(DefaultTool *parent)
        : KoToolSelection(parent), m_selection(parent->koSelection())
    {
        Q_ASSERT(m_selection);
    }

    bool hasSelection() {
        return m_selection->count();
    }

private:
    KoSelection *m_selection;
};


DefaultTool::DefaultTool( KoCanvasBase *canvas )
    : KoInteractionTool( canvas ),
    m_lastHandle(KoFlake::NoHandle),
    m_hotPosition( KoFlake::TopLeftCorner ),
    m_mouseWasInsideHandles( false ),
    m_moveCommand(0),
    m_selectionHandler(new SelectionHandler(this)),
    m_customEventStrategy(0)
{
    setupActions();

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
    m_sizeCursors[0] = Qt::SizeVerCursor;
    m_sizeCursors[1] = Qt::SizeBDiagCursor;
    m_sizeCursors[2] = Qt::SizeHorCursor;
    m_sizeCursors[3] = Qt::SizeFDiagCursor;
    m_sizeCursors[4] = Qt::SizeVerCursor;
    m_sizeCursors[5] = Qt::SizeBDiagCursor;
    m_sizeCursors[6] = Qt::SizeHorCursor;
    m_sizeCursors[7] = Qt::SizeFDiagCursor;

    KoShapeManager * manager = canvas->shapeManager();
    connect( manager, SIGNAL( selectionChanged() ), this, SLOT( updateActions() ) );
}

DefaultTool::~DefaultTool()
{
}

bool DefaultTool::wantsAutoScroll()
{
    return true;
}

void DefaultTool::setupActions()
{
    QAction* actionBringToFront = new QAction( KIcon( "bring_forward" ),
                                               i18n( "Bring to &Front" ), this );
    addAction( "object_move_totop", actionBringToFront );
    actionBringToFront->setShortcut( QKeySequence( "Ctrl+Shift+]" ) );
    connect(actionBringToFront, SIGNAL(triggered()), this, SLOT(selectionBringToFront()));

    QAction* actionRaise = new QAction( KIcon( "raise" ), i18n( "&Raise" ), this );
    addAction( "object_move_up", actionRaise );
    actionRaise->setShortcut( QKeySequence( "Ctrl+]" ) );
    connect(actionRaise, SIGNAL(triggered()), this, SLOT(selectionMoveUp()));

    QAction* actionLower = new QAction( KIcon( "lower" ), i18n( "&Lower" ), this );
    addAction( "object_move_down", actionLower );
    actionLower->setShortcut( QKeySequence( "Ctrl+[" ) );
    connect(actionLower, SIGNAL(triggered()), this, SLOT(selectionMoveDown()));

    QAction* actionSendToBack = new QAction( KIcon( "send_backward" ),
                                             i18n( "Send to &Back" ), this );
    addAction( "object_move_tobottom", actionSendToBack );
    actionSendToBack->setShortcut( QKeySequence( "Ctrl+Shift+[" ) );
    connect(actionSendToBack, SIGNAL(triggered()), this, SLOT(selectionSendToBack()));

    QAction* actionAlignLeft = new QAction( KIcon( "aoleft" ),
                                            i18n( "Align Left" ), this );
    addAction( "object_align_horizontal_left", actionAlignLeft );
    connect(actionAlignLeft, SIGNAL(triggered()), this, SLOT(selectionAlignHorizontalLeft()));

    QAction* actionAlignCenter = new QAction( KIcon( "aocenterh" ),
                                              i18n( "Horizontally Center" ), this );
    addAction( "object_align_horizontal_center", actionAlignCenter );
    connect(actionAlignCenter, SIGNAL(triggered()), this, SLOT(selectionAlignHorizontalCenter()));

    QAction* actionAlignRight = new QAction( KIcon( "aoright" ),
                                             i18n( "Align Right" ), this );
    addAction( "object_align_horizontal_right", actionAlignRight );
    connect(actionAlignRight, SIGNAL(triggered()), this, SLOT(selectionAlignHorizontalRight()));

    QAction* actionAlignTop = new QAction( KIcon( "aotop" ), i18n( "Align Top" ), this );
    addAction( "object_align_vertical_top", actionAlignTop );
    connect(actionAlignTop, SIGNAL(triggered()), this, SLOT(selectionAlignVerticalTop()));

    QAction* actionAlignMiddle = new QAction( KIcon( "aocenterv" ),
                                              i18n( "Vertically Center" ), this );
    addAction( "object_align_vertical_center", actionAlignMiddle );
    connect(actionAlignMiddle, SIGNAL(triggered()), this, SLOT(selectionAlignVerticalCenter()));

    QAction* actionAlignBottom = new QAction( KIcon( "aobottom" ),
                                              i18n( "Align Bottom" ), this );
    addAction( "object_align_vertical_bottom", actionAlignBottom );
    connect(actionAlignBottom, SIGNAL(triggered()), this, SLOT(selectionAlignVerticalBottom()));
}

double DefaultTool::rotationOfHandle( KoFlake::SelectionHandle handle, bool useEdgeRotation )
{
    QPointF selectionCenter = koSelection()->absolutePosition();
    QPointF direction;

    switch( handle )
    {
        case KoFlake::TopMiddleHandle:
            if( useEdgeRotation )
            {
                direction = koSelection()->absolutePosition( KoFlake::TopRightCorner )
                    - koSelection()->absolutePosition( KoFlake::TopLeftCorner );
            }
            else
            {
                QPointF handlePosition = koSelection()->absolutePosition( KoFlake::TopLeftCorner );
                handlePosition += 0.5 * ( koSelection()->absolutePosition( KoFlake::TopRightCorner ) - handlePosition );
                direction = handlePosition - selectionCenter;
            }
            break;
        case KoFlake::TopRightHandle:
            direction = koSelection()->absolutePosition( KoFlake::TopRightCorner ) - selectionCenter;
            break;
        case KoFlake::RightMiddleHandle:
            if( useEdgeRotation )
            {
                direction = koSelection()->absolutePosition( KoFlake::BottomRightCorner )
                        - koSelection()->absolutePosition( KoFlake::TopRightCorner );
            }
            else
            {
                QPointF handlePosition = koSelection()->absolutePosition( KoFlake::TopRightCorner );
                handlePosition += 0.5 * ( koSelection()->absolutePosition( KoFlake::BottomRightCorner ) - handlePosition );
                direction = handlePosition - selectionCenter;
            }
            break;
        case KoFlake::BottomRightHandle:
            direction = koSelection()->absolutePosition( KoFlake::BottomRightCorner ) - selectionCenter;
            break;
        case KoFlake::BottomMiddleHandle:
            if( useEdgeRotation )
            {
                direction = koSelection()->absolutePosition( KoFlake::BottomLeftCorner )
                        - koSelection()->absolutePosition( KoFlake::BottomRightCorner );
            }
            else
            {
                QPointF handlePosition = koSelection()->absolutePosition( KoFlake::BottomLeftCorner );
                handlePosition += 0.5 * ( koSelection()->absolutePosition( KoFlake::BottomRightCorner ) - handlePosition );
                direction = handlePosition - selectionCenter;
            }
            break;
        case KoFlake::BottomLeftHandle:
            direction = koSelection()->absolutePosition( KoFlake::BottomLeftCorner ) - selectionCenter;
            break;
        case KoFlake::LeftMiddleHandle:
            if( useEdgeRotation )
            {
                direction = koSelection()->absolutePosition( KoFlake::TopLeftCorner )
                        - koSelection()->absolutePosition( KoFlake::BottomLeftCorner );
            }
            else
            {
                QPointF handlePosition = koSelection()->absolutePosition( KoFlake::TopLeftCorner );
                handlePosition += 0.5 * ( koSelection()->absolutePosition( KoFlake::BottomLeftCorner ) - handlePosition );
                direction = handlePosition - selectionCenter;
            }
            break;
        case KoFlake::TopLeftHandle:
            direction = koSelection()->absolutePosition( KoFlake::TopLeftCorner ) - selectionCenter;
            break;
        case KoFlake::NoHandle:
            return 0.0;
            break;
    }

    double rotation = atan2( direction.y(), direction.x() ) * 180.0 / M_PI;

    switch( handle )
    {
        case KoFlake::TopMiddleHandle:
            if( useEdgeRotation )
                rotation -= 0.0;
            else
                rotation -= 270.0;
            break;
        case KoFlake::TopRightHandle:
            rotation -= 315.0;
            break;
        case KoFlake::RightMiddleHandle:
            if( useEdgeRotation )
                rotation -= 90.0;
            else
                rotation -= 0.0;
            break;
        case KoFlake::BottomRightHandle:
            rotation -= 45.0;
            break;
        case KoFlake::BottomMiddleHandle:
            if( useEdgeRotation )
                rotation -= 180.0;
            else
                rotation -= 90.0;
            break;
        case KoFlake::BottomLeftHandle:
            rotation -= 135.0;
            break;
        case KoFlake::LeftMiddleHandle:
            if( useEdgeRotation )
                rotation -= 270.0;
            else
                rotation -= 180.0;
            break;
        case KoFlake::TopLeftHandle:
            rotation -= 225.0;
            break;
        case KoFlake::NoHandle:
            break;
    }

    if( rotation < 0.0 )
        rotation += 360.0;

    return rotation;
}

void DefaultTool::updateCursor() {
    QCursor cursor = Qt::ArrowCursor;

    if(koSelection()->count() > 0) { // has a selection
        bool editable=false;
        // check if we have at least one shape that is edtiable
        foreach(KoShape *shape, koSelection()->selectedShapes(KoFlake::StrippedSelection)) {
            if( shape->isEditable() ) {
                editable = true;
                break;
            }
        }

        if(!m_mouseWasInsideHandles) {
            m_angle = rotationOfHandle( m_lastHandle, true );
            int rotOctant = 8 + int(8.5 + m_angle / 45);

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
            m_angle = rotationOfHandle( m_lastHandle, false );
            int rotOctant = 8 + int(8.5 + m_angle / 45);
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

void DefaultTool::paint( QPainter &painter, const KoViewConverter &converter) {
    KoInteractionTool::paint(painter, converter);
    if ( m_currentStrategy  == 0 && koSelection()->count() > 0) {
        SelectionDecorator decorator(m_mouseWasInsideHandles ? m_lastHandle : KoFlake::NoHandle,
                 true, true);
        decorator.setSelection(koSelection());
        decorator.setHandleRadius( m_canvas->resourceProvider()->handleRadius() );
        decorator.setHotPosition( m_hotPosition );
        decorator.paint(painter, converter);
    }
    painter.save();
    KoShape::applyConversion( painter, converter );
    m_canvas->snapGuide()->paint( painter, converter );
    painter.restore();
}

void DefaultTool::mousePressEvent( KoPointerEvent *event ) {
    KoInteractionTool::mousePressEvent(event);
    updateCursor();
}

void DefaultTool::mouseMoveEvent( KoPointerEvent *event ) {
    KoInteractionTool::mouseMoveEvent(event);
    if(m_currentStrategy == 0 && koSelection()->count() > 0) {
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
    updateCursor();
}

QRectF DefaultTool::handlesSize() {
    QRectF bound = koSelection()->boundingRect();
    // expansion Border
    if ( !m_canvas || !m_canvas->viewConverter() ) return bound;

    QPointF border = m_canvas->viewConverter()->viewToDocument(QPointF(HANDLE_DISTANCE, HANDLE_DISTANCE));
    bound.adjust(-border.x(), -border.y(), border.x(), border.y());
    return bound;
}

void DefaultTool::mouseReleaseEvent( KoPointerEvent *event ) {
    KoInteractionTool::mouseReleaseEvent(event);
    updateCursor();
}

void DefaultTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
    QList<KoShape*> shapes;
    foreach(KoShape *shape, koSelection()->selectedShapes()) {
        if(shape->boundingRect().contains(event->point) && // first 'cheap' check
                shape->outline().contains(event->point)) // this is more expensive but weeds out the almost hits
            shapes.append(shape);
    }
    if(shapes.count() == 0) { // nothing in the selection was clicked on.
        KoShape *shape = m_canvas->shapeManager()->shapeAt (event->point, KoFlake::ShapeOnTop);
        if(shape)
            shapes.append(shape);
    }

    KoToolManager::instance()->switchToolRequested(
            KoToolManager::instance()->preferredToolForSelection(shapes));
}

bool DefaultTool::moveSelection( int direction, Qt::KeyboardModifiers modifiers )
{
    double x=0.0, y=0.0;
    if(direction == Qt::Key_Left)
        x=-0.5;
    else if(direction == Qt::Key_Right)
        x=0.5;
    else if(direction == Qt::Key_Up)
        y=-0.5;
    else if(direction == Qt::Key_Down)
        y=0.5;

    if(x != 0.0 || y != 0.0) { // actually move
        if((modifiers & Qt::ShiftModifier) == 0) { // no shift used
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
            return true;
        }
    }
    return false;
}

void DefaultTool::keyPressEvent(QKeyEvent *event) {
    KoInteractionTool::keyPressEvent(event);
    if(m_currentStrategy == 0) {
        switch( event->key() )
        {
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
                if( moveSelection( event->key(), event->modifiers() ) )
                    event->accept();
                break;
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
                m_canvas->resourceProvider()->setResource( KoCanvasResource::HotPosition, event->key()-Qt::Key_1 );
                event->accept();
                break;
            default:
                return;
        }
    }
}

void DefaultTool::customMoveEvent( KoPointerEvent * event )
{
    if( ! koSelection()->count() )
    {
        event->ignore();
        return;
    }

    int move = qMax( qAbs(event->x()), qAbs(event->y() ) );
    int zoom = qAbs(event->z());
    int rotate = qAbs(event->rotationZ());
    const int threshold = 2;

    if( move < threshold && zoom < threshold && rotate < threshold )
    {
        if( m_customEventStrategy )
        {
            m_customEventStrategy->finishInteraction( event->modifiers() );
            QUndoCommand *command = m_customEventStrategy->createCommand();
            if(command)
                m_canvas->addCommand(command);
            delete m_customEventStrategy;
            m_customEventStrategy = 0;
            repaintDecorations();
        }
        event->accept();
        return;
    }

    // check if the z-movement is dominant
    if( zoom > move && zoom > rotate )
    {
        // zoom
        if( ! m_customEventStrategy )
            m_customEventStrategy = new ShapeResizeStrategy( this, m_canvas, event->point, KoFlake::TopLeftHandle );
    }
    // check if x-/y-movement is dominant
    else if( move > zoom && move > rotate )
    {
        // move
        if( ! m_customEventStrategy )
            m_customEventStrategy = new ShapeMoveStrategy( this, m_canvas, event->point );
    }
    // rotation is dominant
    else if( rotate > zoom && rotate > move )
    {
        // rotate
        if( ! m_customEventStrategy )
            m_customEventStrategy = new ShapeRotateStrategy( this, m_canvas, event->point, event->buttons() );
    }

    if( m_customEventStrategy )
        m_customEventStrategy->handleCustomEvent( event );

    event->accept();
}

void DefaultTool::repaintDecorations() {
    Q_ASSERT(koSelection());
    if ( koSelection()->count() > 0 )
        m_canvas->updateCanvas(handlesSize());
}

void DefaultTool::copy() const
{
    QList<KoShape *> shapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::StrippedSelection );
    if ( !shapes.empty() ) {
        KoShapeOdfSaveHelper saveHelper( shapes );
        KoDrag drag;
        drag.setOdf( KoOdf::mimeType( KoOdf::Text ), saveHelper );
        drag.addToClipboard();
    }
}

void DefaultTool::deleteSelection()
{
    // tz: TODO is StrippedSelection the right one?
    QList<KoShape *> shapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::StrippedSelection );
    if ( !shapes.empty() ) {
        m_canvas->addCommand( m_canvas->shapeController()->removeShapes(shapes) );
    }
}

bool DefaultTool::paste()
{
    const QMimeData * data = QApplication::clipboard()->mimeData();

    bool success = false;
    if ( data->hasFormat( KoOdf::mimeType( KoOdf::Text ) ) ) {
        KoShapeManager * shapeManager = m_canvas->shapeManager();
        int zIndex = 0;
        foreach ( KoShape *shape, shapeManager->shapes() )
        {
            zIndex = qMax( zIndex, shape->zIndex() );
        }

        KoShapePaste paste( m_canvas, zIndex + 1, shapeManager->selection()->activeLayer() );
        success = paste.paste( KoOdf::Text, data );
    }
    return success;
}

QStringList DefaultTool::supportedPasteMimeTypes() const
{
    QStringList list;
    list << KoOdf::mimeType( KoOdf::Text );
    return list;
}

KoSelection *DefaultTool::koSelection() {
    Q_ASSERT(m_canvas);
    Q_ASSERT(m_canvas->shapeManager());
    return m_canvas->shapeManager()->selection();
}

KoFlake::SelectionHandle DefaultTool::handleAt(const QPointF &point, bool *innerHandleMeaning) {
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
    const KoViewConverter *converter = m_canvas->viewConverter();
    if ( !converter ) return KoFlake::NoHandle;

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

void DefaultTool::recalcSelectionBox() {
    if(koSelection()->count()==0)
        return;

    if(koSelection()->count()>1)
    {
        QMatrix matrix = koSelection()->absoluteTransformation(0);
        m_selectionOutline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), koSelection()->size())));
        m_angle = 0.0; //koSelection()->rotation();
    }
    else
    {
        QMatrix matrix = koSelection()->firstSelectedShape()->absoluteTransformation(0);
        m_selectionOutline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), koSelection()->firstSelectedShape()->size())));
        m_angle = 0.0; //koSelection()->firstSelectedShape()->rotation();
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
    if (koSelection()->count() == 1) {
#if 0        // TODO detect mirroring
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
#endif
    }
}

void DefaultTool::activate(bool temporary) {
    Q_UNUSED(temporary);
    m_mouseWasInsideHandles = false;
    m_lastHandle = KoFlake::NoHandle;
    useCursor(Qt::ArrowCursor, true);
    repaintDecorations();
}

void DefaultTool::selectionAlignHorizontalLeft() {

    selectionAlign(KoShapeAlignCommand::HorizontalLeftAlignment);
}

void DefaultTool::selectionAlignHorizontalCenter() {

    selectionAlign(KoShapeAlignCommand::HorizontalCenterAlignment);
}


void DefaultTool::selectionAlignHorizontalRight() {

    selectionAlign(KoShapeAlignCommand::HorizontalRightAlignment);
}

void DefaultTool::selectionAlignVerticalTop() {

    selectionAlign(KoShapeAlignCommand::VerticalTopAlignment);
}

void DefaultTool::selectionAlignVerticalCenter() {

    selectionAlign(KoShapeAlignCommand::VerticalCenterAlignment);
}

void DefaultTool::selectionAlignVerticalBottom() {

    selectionAlign(KoShapeAlignCommand::VerticalBottomAlignment);
}


void DefaultTool::selectionAlign(KoShapeAlignCommand::Align align)
{
    KoSelection* selection = m_canvas->shapeManager()->selection();
    if( ! selection )
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::TopLevelSelection );
    if( selectedShapes.count() < 1)
        return;

    QRectF bb;

    foreach( KoShape * shape, selectedShapes )
    {
        if( dynamic_cast<KoShapeGroup*>( shape ) )
            continue;
        if( bb.isNull() )
            bb = shape->absoluteTransformation(0).map( shape->outline() ).boundingRect();
        else
            bb = bb.unite( shape->absoluteTransformation(0).map( shape->outline() ).boundingRect() );
    }

    // TODO add an option to the widget so that one can align to the page
    // with multiple selected shapes too

    // single selected shape is automatically aligned to document rect
    if( selectedShapes.count() == 1 && m_canvas->resourceProvider()->hasResource( KoCanvasResource::PageSize ) )
        bb = QRectF( QPointF(0,0), m_canvas->resourceProvider()->sizeResource( KoCanvasResource::PageSize ) );

    KoShapeAlignCommand *cmd = new KoShapeAlignCommand( selectedShapes, align, bb );

    m_canvas->addCommand( cmd );
    selection->updateSizeAndPosition();
}

void DefaultTool::selectionBringToFront()
{
    selectionReorder( KoShapeReorderCommand::BringToFront );
}

void DefaultTool::selectionMoveUp()
{
    selectionReorder( KoShapeReorderCommand::RaiseShape );
}

void DefaultTool::selectionMoveDown()
{
    selectionReorder( KoShapeReorderCommand::LowerShape );
}

void DefaultTool::selectionSendToBack()
{
    selectionReorder( KoShapeReorderCommand::SendToBack );
}

void DefaultTool::selectionReorder(KoShapeReorderCommand::MoveShapeType order )
{
    KoSelection* selection = m_canvas->shapeManager()->selection();
    if( ! selection )
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes( KoFlake::TopLevelSelection );
    if( selectedShapes.count() < 1)
        return;

    QUndoCommand * cmd = KoShapeReorderCommand::createCommand( selectedShapes, m_canvas->shapeManager(), order );
    m_canvas->addCommand( cmd );
}

QWidget* DefaultTool::createOptionWidget() {
    return new DefaultToolWidget( this );
}

void DefaultTool::resourceChanged( int key, const QVariant & res )
{
    if( key == KoCanvasResource::HotPosition )
    {
        m_hotPosition = static_cast<KoFlake::Position>( res.toInt() );
        repaintDecorations();
    }
}

KoInteractionStrategy *DefaultTool::createStrategy(KoPointerEvent *event) {

    KoShapeManager *shapeManager = m_canvas->shapeManager();
    KoSelection *select = shapeManager->selection();
    bool insideSelection, editableShape=false;
    KoFlake::SelectionHandle handle = handleAt(event->point, &insideSelection);

    foreach (KoShape* shape, select->selectedShapes()) {
        if( shape->isEditable() ) {
            editableShape = true;
            break;
        }
    }

    if( event->buttons() & Qt::MidButton )
    {
        // change the hot selection position when middle clicking on a handle
        KoFlake::Position newHotPosition = m_hotPosition;
        switch( handle )
        {
            case KoFlake::TopLeftHandle:
                newHotPosition = KoFlake::TopLeftCorner;
                break;
            case KoFlake::TopRightHandle:
                newHotPosition = KoFlake::TopRightCorner;
                break;
            case KoFlake::BottomLeftHandle:
                newHotPosition = KoFlake::BottomLeftCorner;
                break;
            case KoFlake::BottomRightHandle:
                newHotPosition = KoFlake::BottomRightCorner;
                break;
            default:
            {
                // check if we had hit the center point
                const KoViewConverter * converter = m_canvas->viewConverter();
                QPointF pt = converter->documentToView(event->point-select->absolutePosition());
                if( qAbs(pt.x()) < HANDLE_DISTANCE && qAbs(pt.y()) < HANDLE_DISTANCE)
                    newHotPosition = KoFlake::CenteredPosition;
                break;
            }
        }
        if( m_hotPosition != newHotPosition )
            m_canvas->resourceProvider()->setResource( KoCanvasResource::HotPosition, newHotPosition );
        return 0;
    }

    if(editableShape) {
        // manipulation of selected shapes goes first
        if(handle != KoFlake::NoHandle) {
            if( event->buttons() == Qt::LeftButton ) {
                // resizing or shearing only with left mouse button
                if(insideSelection)
                    return new ShapeResizeStrategy(this, m_canvas, event->point, handle);
                if(handle == KoFlake::TopMiddleHandle || handle == KoFlake::RightMiddleHandle ||
                        handle == KoFlake::BottomMiddleHandle || handle == KoFlake::LeftMiddleHandle)
                    return new ShapeShearStrategy(this, m_canvas, event->point, handle);
            }
            // rotating is allowed for rigth mouse button too
            if( handle == KoFlake::TopLeftHandle || handle == KoFlake::TopRightHandle ||
                handle == KoFlake::BottomLeftHandle || handle == KoFlake::BottomRightHandle )
                return new ShapeRotateStrategy(this, m_canvas, event->point, event->buttons() );
        }
        // This is wrong now when there is a single rotated object as you get it also when pressing outside of the object
        if(event->buttons() == Qt::LeftButton && select->boundingRect().contains(event->point))
            return new ShapeMoveStrategy(this, m_canvas, event->point);
    }

    if((event->buttons() & Qt::LeftButton) == 0)
        return 0;  // Nothing to do for middle/right mouse button

    KoShape * object( shapeManager->shapeAt( event->point, (event->modifiers() & Qt::ShiftModifier) ? KoFlake::NextUnselected : KoFlake::ShapeOnTop ) );
    if( !object && handle == KoFlake::NoHandle) {
        if ( ( event->modifiers() & Qt::ControlModifier ) == 0 )
        {
            repaintDecorations();
            select->deselectAll();
        }
        return new KoShapeRubberSelectStrategy(this, m_canvas, event->point);
    }

    if(select->isSelected(object)) {
        if ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier ) {
            repaintDecorations();
            select->deselect(object);
        }
    }
    else if(handle == KoFlake::NoHandle) { // clicked on object which is not selected
        repaintDecorations();
        if ( ( event->modifiers() & Qt::ControlModifier ) == 0 )
            shapeManager->selection()->deselectAll();
        select->select(object);
        repaintDecorations();
        return new ShapeMoveStrategy(this, m_canvas, event->point);
    }
    return 0;
}

void DefaultTool::updateActions()
{
    if (!koSelection())
    {
        action( "object_move_totop" )->setEnabled(false);
        action( "object_move_up" )->setEnabled(false);
        action( "object_move_down" )->setEnabled(false);
        action( "object_move_tobottom" )->setEnabled(false);
        action( "object_align_horizontal_left" )->setEnabled(false);
        action( "object_align_horizontal_center" )->setEnabled(false);
        action( "object_align_horizontal_right" )->setEnabled(false);
        action( "object_align_vertical_top" )->setEnabled(false);
        action( "object_align_vertical_center" )->setEnabled(false);
        action( "object_align_vertical_bottom" )->setEnabled(false);
        return;
    }

    bool enable = koSelection()->count () > 0;
    action( "object_move_totop" )->setEnabled(enable);
    action( "object_move_up" )->setEnabled(enable);
    action( "object_move_down" )->setEnabled(enable);
    action( "object_move_tobottom" )->setEnabled(enable);
    enable = koSelection()->count () > 1;
    action( "object_align_horizontal_left" )->setEnabled(enable);
    action( "object_align_horizontal_center" )->setEnabled(enable);
    action( "object_align_horizontal_right" )->setEnabled(enable);
    action( "object_align_vertical_top" )->setEnabled(enable);
    action( "object_align_vertical_center" )->setEnabled(enable);
    action( "object_align_vertical_bottom" )->setEnabled(enable);

    emit selectionChanged(koSelection()->count());
}

KoToolSelection* DefaultTool::selection()
{
    return m_selectionHandler;
}

#include "DefaultTool.moc"
