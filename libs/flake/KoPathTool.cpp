/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPathTool.h"
#include "KoCanvasBase.h"
#include "KoSelection.h"
#include "KoShapeManager.h"
#include "KoPointerEvent.h"
#include "commands/KoPathPointTypeCommand.h"
#include "commands/KoPathPointInsertCommand.h"
#include "commands/KoPathPointRemoveCommand.h"
#include "commands/KoPathSegmentTypeCommand.h"
#include "commands/KoPathBreakAtPointCommand.h"
#include "commands/KoPathSegmentBreakCommand.h"
#include "commands/KoParameterToPathCommand.h"
#include "commands/KoSubpathJoinCommand.h"
#include "KoParameterShape.h"
#include "KoInsets.h"
#include "KoShapeBorderModel.h"
#include "KoPathControlPointMoveStrategy.h"
#include "KoParameterChangeStrategy.h"
#include "KoPathPointMoveStrategy.h"
#include "KoPathPointRubberSelectStrategy.h"
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <QKeyEvent>
#include <QGridLayout>
#include <QButtonGroup>
#include <QToolButton>
#include <QPainterPath>

KoPathTool::KoPathTool(KoCanvasBase *canvas)
: KoTool(canvas)
, m_activeHandle( 0 )    
, m_handleRadius( 3 )
, m_pointSelection( this )    
, m_currentStrategy(0)
{
}

KoPathTool::~KoPathTool() {
}

QWidget * KoPathTool::createOptionWidget() {
    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout( optionWidget );
    m_pointTypeGroup = new QButtonGroup( optionWidget );
    m_pointTypeGroup->setExclusive( true );

    QToolButton *button = 0;

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathpoint-corner") );
    button->setToolTip( i18n( "Corner point" ) );
    m_pointTypeGroup->addButton( button, KoPathPointTypeCommand::Corner );
    layout->addWidget( button, 0, 0 );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathpoint-smooth") );
    button->setToolTip( i18n( "Smooth point" ) );
    m_pointTypeGroup->addButton( button, KoPathPointTypeCommand::Smooth );
    layout->addWidget( button, 0, 1 );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathpoint-symmetric") );
    button->setToolTip( i18n( "Symmetric point" ) );
    m_pointTypeGroup->addButton( button, KoPathPointTypeCommand::Symmetric );
    layout->addWidget( button, 0, 2 );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathsegment-line") );
    button->setToolTip( i18n( "Segment to line" ) );
    layout->addWidget( button, 0, 4 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( segmentToLine() ) );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathsegment-curve") );
    button->setToolTip( i18n( "Segment to curve" ) );
    layout->addWidget( button, 0, 5 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( segmentToCurve() ) );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("convert-to-path") );
    button->setToolTip( i18n( "Convert to path" ) );
    layout->addWidget( button, 0, 6 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( convertToPath() ) );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathpoint-insert") );
    button->setToolTip( i18n( "Insert point" ) );
    layout->addWidget( button, 1, 0 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( insertPoints() ) );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathpoint-remove") );
    button->setToolTip( i18n( "Remove point" ) );
    layout->addWidget( button, 1, 1 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( removePoints() ) );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("path-break-point", 22) );
    button->setToolTip( i18n( "Break at point" ) );
    layout->addWidget( button, 1, 3 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( breakAtPoint() ) );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("path-break-segment") );
    button->setToolTip( i18n( "Break at segment" ) );
    layout->addWidget( button, 1, 4 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( breakAtSegment() ) );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathpoint-join") );
    button->setToolTip( i18n( "Join with segment" ) );
    layout->addWidget( button, 1, 5 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( joinPoints() ) );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathpoint-merge") );
    button->setToolTip( i18n( "Merge points" ) );
    layout->addWidget( button, 1, 6 );
    //connect( button, SIGNAL( clicked( bool ) ), this, SLOT( removePoints() ) );

    layout->setColumnStretch( 0, 0 );
    layout->setColumnStretch( 1, 0 );
    layout->setColumnStretch( 2, 0 );
    //layout->setColumnMinimumWidth( 3, 10 );
    layout->setColumnStretch( 4, 0 );
    layout->setColumnStretch( 5, 0 );
    layout->setColumnStretch( 6, 0 );
    layout->setColumnStretch( 7, 1 );

    connect( m_pointTypeGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( slotPointTypeChanged( int ) ) );
    return optionWidget;
}

void KoPathTool::slotPointTypeChanged( int type ) 
{
    if ( m_pointSelection.size() > 0 )
    {
        QList<KoPathPointData> selectedPoints = m_pointSelection.selectedPointsData();
        QList<KoPathPointData> pointToChange;

        QList<KoPathPointData>::const_iterator it( selectedPoints.begin() );
        for ( ; it != selectedPoints.end(); ++it )
        {
            KoPathPoint *point = it->m_pathShape->pointByIndex( it->m_pointIndex );
            if ( point )
            {
                if ( point->activeControlPoint1() && point->activeControlPoint2() )
                {
                    pointToChange.append( *it );
                }
            }
        }

        if ( pointToChange.size() > 0 )
        {
            KoPathPointTypeCommand *cmd = new KoPathPointTypeCommand( pointToChange, (KoPathPointTypeCommand::PointType)type );
            m_canvas->addCommand( cmd );
        }
    }
}

void KoPathTool::insertPoints() 
{
    if ( m_pointSelection.size() > 1 )
    {
        QList<KoPathPointData> segments( m_pointSelection.selectedSegmentsData() );
        if ( segments.size() > 0 )
        {
            KoPathPointInsertCommand *cmd = new KoPathPointInsertCommand( segments, 0.5 );
            m_canvas->addCommand( cmd );
        }
    }
}

void KoPathTool::removePoints() {
    // TODO finish current action or should this not possible during actions???
    if ( m_pointSelection.size() > 0 )
    {
        QUndoCommand *cmd = KoPathPointRemoveCommand::createCommand( m_pointSelection.selectedPointsData(), m_canvas->shapeController() );
        ActivePointHandle *pointHandle = dynamic_cast<ActivePointHandle*>( m_activeHandle );
        if ( pointHandle && m_pointSelection.contains( pointHandle->m_activePoint ) )
        {
            delete m_activeHandle;
            m_activeHandle = 0;
        }
        m_pointSelection.clear();
        m_canvas->addCommand( cmd );
    }
}

void KoPathTool::segmentToLine() 
{
    if ( m_pointSelection.size() > 1 )
    {
        QList<KoPathPointData> segments( m_pointSelection.selectedSegmentsData() );
        if ( segments.size() > 0 )
        {
            m_canvas->addCommand( new KoPathSegmentTypeCommand( segments, KoPathSegmentTypeCommand::Line ) );
        }
    }
}

void KoPathTool::segmentToCurve() 
{
    if ( m_pointSelection.size() > 1 )
    {
        QList<KoPathPointData> segments( m_pointSelection.selectedSegmentsData() );
        if ( segments.size() > 0 )
        {
            m_canvas->addCommand( new KoPathSegmentTypeCommand( segments, KoPathSegmentTypeCommand::Curve ) );
        }
    }
}

void KoPathTool::convertToPath()
{
    QList<KoParameterShape*> shapesToConvert;
    foreach( KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection ) )
    {
        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( shape );
        if ( parameterShape && parameterShape->isParametricShape() )
            shapesToConvert.append( parameterShape );
    }
    if( shapesToConvert.count() )
        m_canvas->addCommand( new KoParameterToPathCommand( shapesToConvert ) );
}

void KoPathTool::joinPoints()
{
    if ( m_pointSelection.objectCount() == 1 && m_pointSelection.size() == 2 )
    {
        QList<KoPathPointData> pd( m_pointSelection.selectedPointsData() );
        const KoPathPointData & pd1 = pd.at( 0 );
        const KoPathPointData & pd2 = pd.at( 1 );
        KoPathShape * pathShape = pd1.m_pathShape;
        if ( !pathShape->isClosedSubpath( pd1.m_pointIndex.first ) &&  
            ( pd1.m_pointIndex.second == 0 || 
              pd1.m_pointIndex.second == pathShape->pointCountSubpath( pd1.m_pointIndex.first ) - 1 ) &&
             !pathShape->isClosedSubpath( pd2.m_pointIndex.first ) && 
            ( pd2.m_pointIndex.second == 0 || 
              pd2.m_pointIndex.second == pathShape->pointCountSubpath( pd2.m_pointIndex.first ) - 1 ) )
        {
            KoSubpathJoinCommand *cmd = new KoSubpathJoinCommand( pd1, pd2 );
            m_canvas->addCommand( cmd );
        }
    }
}

void KoPathTool::breakAtPoint()
{
    if ( m_pointSelection.size() > 0 )
    {
        m_canvas->addCommand( new KoPathBreakAtPointCommand( m_pointSelection.selectedPointsData() ) );
    }
}

void KoPathTool::breakAtSegment()
{
    // only try to break a segment when 2 points of the same object are selected
    if ( m_pointSelection.objectCount() == 1 && m_pointSelection.size() == 2 )
    {
        QList<KoPathPointData> segments( m_pointSelection.selectedSegmentsData() );
        if ( segments.size() == 1 )
        {
            m_canvas->addCommand( new KoPathSegmentBreakCommand( segments.at( 0 ) ) );
        }
    }
}

void KoPathTool::paint( QPainter &painter, KoViewConverter &converter) {
    QList<KoShape*> selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );

    painter.save();
    painter.setRenderHint( QPainter::Antialiasing, true );
    // use different colors so that it is also visible on a background of the same color
    painter.setBrush( Qt::white ); //TODO make configurable
    painter.setPen( Qt::blue );

    foreach( KoShape *shape, selectedShapes ) 
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>( shape );

        if ( !shape->isLocked() && pathShape )
        {
            painter.save();
            painter.setMatrix( shape->transformationMatrix( &converter ) * painter.matrix() );

            KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( shape );
            if ( parameterShape && parameterShape->isParametricShape() )
            {
                parameterShape->paintHandles( painter, converter, m_handleRadius );
            }
            else
            {
                pathShape->paintPoints( painter, converter, m_handleRadius );
            }

            painter.restore();
        }
    }

    painter.restore();

    if ( m_currentStrategy )
    {
        painter.save();
        m_currentStrategy->paint( painter, converter );
        painter.restore();
    }

    painter.save();
    painter.setBrush( Qt::green ); // TODO make color configurable
    painter.setPen( Qt::blue );

    m_pointSelection.update();
    m_pointSelection.paint( painter, converter );

    painter.setBrush( Qt::red ); // TODO make color configurable
    painter.setPen( Qt::blue );

    if ( m_activeHandle )
    {
        if ( m_activeHandle->check() ) 
        {
            m_activeHandle->paint( painter, converter );
        }
        else
        {
            delete m_activeHandle;
            m_activeHandle = 0;
        }
    }

    painter.restore();
}

void KoPathTool::repaintDecorations()
{
    foreach(KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes()) 
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*> (shape);

        if ( !shape->isLocked() && pathShape )
        {
            repaint( pathShape->boundingRect() );
        }
    }

    m_pointSelection.repaint();
}

void KoPathTool::mousePressEvent( KoPointerEvent *event ) {
    // we are moving if we hit a point and use the left mouse button
    if ( m_activeHandle )
    {
        m_activeHandle->mousePressEvent( event );
    }
    else
    {
        if( event->button() & Qt::LeftButton )
        {
            if( event->modifiers() & Qt::ControlModifier == 0 )
            {
                m_pointSelection.clear();
            }
            // start rubberband selection
            Q_ASSERT(m_currentStrategy == 0);
            m_currentStrategy = new KoPathPointRubberSelectStrategy( this, m_canvas, event->point );
        }
    }
}

void KoPathTool::mouseDoubleClickEvent( KoPointerEvent * ) {
}

void KoPathTool::mouseMoveEvent( KoPointerEvent *event ) {
    if( event->button() & Qt::RightButton )
        return;

    // repaint old handle positions
    m_pointSelection.repaint();
    if ( m_activeHandle )
        m_activeHandle->repaint();

    if ( m_currentStrategy )
    {
        m_lastPoint = event->point;
        m_currentStrategy->handleMouseMove( event->point, event->modifiers() );

        // repaint new handle positions
        m_pointSelection.repaint();
        if ( m_activeHandle )
            m_activeHandle->repaint();
        return;
    }

    QList<KoShape*> selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    foreach( KoShape *shape, selectedShapes ) 
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>( shape );

        if ( !shape->isLocked() && pathShape )
        {
            QRectF roi = handleRect( pathShape->documentToShape( event->point ) );
            KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( pathShape );
            if ( parameterShape && parameterShape->isParametricShape() )
            {
                int handleId = parameterShape->handleIdAt( roi );
                //qDebug() << "handleId" << handleId;
                if ( handleId != -1 )
                {
                    useCursor(Qt::SizeAllCursor);
                    delete m_activeHandle;
                    m_activeHandle = new ActiveParameterHandle( this, parameterShape, handleId );
                    m_activeHandle->repaint();
                    return;
                }

            }
            else
            {
                QList<KoPathPoint*> points = pathShape->pointsAt( roi );
                if( ! points.empty() )
                {
                    useCursor(Qt::SizeAllCursor);

                    KoPathPoint *p = points.first();
                    KoPathPoint::KoPointType type = KoPathPoint::Node;

                    // check for the control points as otherwise it is no longer 
                    // possible to change the control points when they are the same as the point
                    if( p->properties() & KoPathPoint::HasControlPoint1 && roi.contains( p->controlPoint1() ) )
                        type = KoPathPoint::ControlPoint1;
                    else if( p->properties() & KoPathPoint::HasControlPoint2 && roi.contains( p->controlPoint2() ) )
                        type = KoPathPoint::ControlPoint2;

                    delete m_activeHandle;
                    m_activeHandle = new ActivePointHandle( this, p, type );
                    m_activeHandle->repaint();
                    return;
                }
            }
        }
    }

    useCursor(Qt::ArrowCursor);
    delete m_activeHandle;
    m_activeHandle = 0;
}

void KoPathTool::mouseReleaseEvent( KoPointerEvent *event ) {
    if ( m_currentStrategy )
    {
        m_currentStrategy->finishInteraction( event->modifiers() );
        QUndoCommand *command = m_currentStrategy->createCommand();
        if ( command )
            m_canvas->addCommand( command );
        delete m_currentStrategy;
        m_currentStrategy = 0;
    }
}

void KoPathTool::keyPressEvent(QKeyEvent *event) {
    if ( m_currentStrategy )
    {
        switch ( event->key() )
        {
            case Qt::Key_Control:
            case Qt::Key_Alt:
            case Qt::Key_Shift:
            case Qt::Key_Meta:
                m_currentStrategy->handleMouseMove( m_lastPoint, event->modifiers() );
                break;
            case Qt::Key_Escape:
                m_currentStrategy->cancelInteraction();
                delete m_currentStrategy;
                m_currentStrategy = 0;
                break;
            default:
                event->ignore();
                return;
        }
    }
    else
    {
        QList<KoPathPoint*> selectedPoints = m_pointSelection.selectedPoints().toList();
        KoPathShape * pathShape = 0;
        if ( m_pointSelection.objectCount() == 1 )
        {
            pathShape = selectedPoints[0]->parent();
        }

        switch(event->key()) 
        {
            case Qt::Key_I:
            {
                int handleRadius = m_canvas->resourceProvider()->handleRadius();
                if(event->modifiers() & Qt::ControlModifier) 
                    handleRadius--;
                else
                    handleRadius++;
                m_canvas->resourceProvider()->setHandleRadius( handleRadius );
                break;
            }
            case Qt::Key_Backspace:
                removePoints();
                break;
            case Qt::Key_Insert:
                insertPoints();
                break;
#ifndef NDEBUG
            case Qt::Key_D:
                if ( pathShape )
                {
                    if ( pathShape )
                    {
                        pathShape->debugPath();
                    }
                }
                break;
#endif
            case Qt::Key_B:
                if( selectedPoints.size() == 1 )
                    breakAtPoint();
                else if( selectedPoints.size() >= 2 )
                    breakAtSegment();
                break;
            case Qt::Key_J:
                joinPoints();
                break;
            case Qt::Key_F:
                segmentToLine();
                break;
            case Qt::Key_C:
                segmentToCurve();
                break;
            case Qt::Key_P:
                convertToPath();
                break;
            default:
                event->ignore();
                return;
        }
    }
    event->accept();
}

void KoPathTool::keyReleaseEvent(QKeyEvent *event) {
    if ( m_currentStrategy )
    {
        switch ( event->key() )
        {
            case Qt::Key_Control:
            case Qt::Key_Alt:
            case Qt::Key_Shift:
            case Qt::Key_Meta:
                m_currentStrategy->handleMouseMove( m_lastPoint, Qt::NoModifier );
                break;
            default:
                break;
        }
    }
    event->accept();
}

void KoPathTool::activate (bool temporary) {
    Q_UNUSED(temporary);
    bool found = false;

    // retrieve the actual global handle radius
    m_handleRadius = m_canvas->resourceProvider()->handleRadius();

    foreach(KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes()) 
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*> (shape);

        if ( !shape->isLocked() && pathShape )
        {
            found = true;
            // as the tool is just in activation repaintDecorations does not yet get called
            // so we need to use repaint of the tool and it is only needed to repaint the 
            // current canvas
            repaint( pathShape->boundingRect() );
        }
    }
    if( !found ) {
        emit sigDone();
        return;
    }
    useCursor(Qt::ArrowCursor, true);
}

void KoPathTool::deactivate() {
    m_pointSelection.clear();

    delete m_activeHandle;
    m_activeHandle = 0;
    delete m_currentStrategy;
    m_currentStrategy = 0;
}

void KoPathTool::resourceChanged( int key, const QVariant & res )
{
    switch( key )
    {
        case KoCanvasResource::HandleRadius:
        {
            QRectF repaintRect;
            foreach(KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes() )
            {
                KoPathShape *pathShape = dynamic_cast<KoPathShape*>( shape );
                if( pathShape )
                {
                    if( repaintRect.isEmpty() )
                        repaintRect = pathShape->shapeToDocument( pathShape->outline().controlPointRect() );
                    else
                        repaintRect |= pathShape->shapeToDocument( pathShape->outline().controlPointRect() );
                }
            }
            repaint( repaintRect );
            m_pointSelection.repaint();
            m_handleRadius = res.toUInt();
            m_pointSelection.repaint();
            repaint( repaintRect );
        }
        break;
        default:
            return;
    }
}

void KoPathTool::selectPoints( const QRectF &rect, bool clearSelection )
{
    if( clearSelection )
    {
        m_pointSelection.clear();
    }

    QList<KoShape*> selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    foreach( KoShape *shape, selectedShapes ) 
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>( shape );

        if ( !shape->isLocked() && pathShape )
        {
            KoParameterShape *parameterShape = dynamic_cast<KoParameterShape*>( shape );

            if ( ! parameterShape || ! parameterShape->isParametricShape() )
            {
                QList<KoPathPoint*> selected = pathShape->pointsAt( pathShape->documentToShape( rect ) );
                foreach( KoPathPoint* point, selected )
                {
                    m_pointSelection.add( point, false );
                }
            }
        }
    }
}

void KoPathTool::repaint( const QRectF &repaintRect ) {
    //qDebug() << "KoPathTool::repaint(" << repaintRect << ")" << m_handleRadius;
    m_canvas->updateCanvas( repaintRect.adjusted( -m_handleRadius, -m_handleRadius, m_handleRadius, m_handleRadius ) );
}

QRectF KoPathTool::handleRect( const QPointF &p ) {
    return QRectF( p.x()-m_handleRadius, p.y()-m_handleRadius, 2*m_handleRadius, 2*m_handleRadius );
}

QPointF KoPathTool::snapToGrid( const QPointF &p, Qt::KeyboardModifiers modifiers ) {
    if( ! m_canvas->snapToGrid() || modifiers & Qt::ShiftModifier )
        return p;

    double gridX, gridY;
    m_canvas->gridSize( &gridX, &gridY );
    return QPointF( static_cast<int>( p.x() / gridX + 1e-10 ) * gridX,
                    static_cast<int>( p.y() / gridY + 1e-10 ) * gridY );
}

void KoPathTool::ActivePointHandle::paint( QPainter &painter, KoViewConverter &converter )
{ 
    painter.save();
    painter.setMatrix( m_activePoint->parent()->transformationMatrix(&converter) * painter.matrix() );
    KoShape::applyConversion( painter, converter );

    QRectF handle = converter.viewToDocument( m_tool->handleRect( QPoint(0,0) ) );
    m_activePoint->paint( painter, handle.size(), m_tool->m_pointSelection.contains( m_activePoint ) ? KoPathPoint::All : KoPathPoint::Node );
    painter.restore();
}

void KoPathTool::ActivePointHandle::repaint() const
{
    m_tool->repaint( m_activePoint->boundingRect() );
}

void KoPathTool::ActivePointHandle::mousePressEvent( KoPointerEvent *event ) 
{
    if( event->button() & Qt::LeftButton )
    {
        // control select adds/removes points to/from the selection
        if( event->modifiers() & Qt::ControlModifier )
        {
            if ( m_tool->m_pointSelection.contains( m_activePoint ) )
            {
                m_tool->m_pointSelection.remove( m_activePoint );
            }
            else
            {
                m_tool->m_pointSelection.add( m_activePoint, false ); 
            }
        }
        else
        {
            // no control modifier, so clear selection and select active point
            if ( !m_tool->m_pointSelection.contains( m_activePoint ) )
            {
                m_tool->m_pointSelection.add( m_activePoint, true ); 
            }
        }
        // TODO remove canvas from call ?
        if ( m_activePointType == KoPathPoint::Node )
        {
            m_tool->m_currentStrategy = new KoPathPointMoveStrategy( m_tool, m_tool->m_canvas, event->point );
        }
        else
        {
            KoPathShape * pathShape = m_activePoint->parent();
            KoPathPointData pd( pathShape, pathShape->pathPointIndex( m_activePoint ) );
            m_tool->m_currentStrategy = new KoPathControlPointMoveStrategy( m_tool, m_tool->m_canvas, pd, m_activePointType, event->point );
        }

    }
    else if( event->button() & Qt::RightButton )
    {
        KoPathPoint::KoPointProperties props = m_activePoint->properties();
        if( (props & KoPathPoint::HasControlPoint1) == 0 || (props & KoPathPoint::HasControlPoint2) == 0 )
            return;

        KoPathPointTypeCommand::PointType pointType = KoPathPointTypeCommand::Smooth;
        // cycle the smooth->symmetric->unsmooth state of the path point
        if( props & KoPathPoint::IsSmooth )
            pointType = KoPathPointTypeCommand::Symmetric;
        else if( props & KoPathPoint::IsSymmetric )
            pointType = KoPathPointTypeCommand::Corner;

        QList<KoPathPointData> pointData;
        pointData.append( KoPathPointData( m_activePoint->parent(), m_activePoint->parent()->pathPointIndex( m_activePoint ) ) );
        m_tool->m_canvas->addCommand( new KoPathPointTypeCommand( pointData, pointType ) );
    }
}

bool KoPathTool::ActivePointHandle::check() 
{
    if ( m_tool->m_canvas->shapeManager()->selection()->isSelected( m_activePoint->parent() ) )
    {
        return m_activePoint->parent()->pathPointIndex( m_activePoint ) != KoPathPointIndex( -1, -1 );
    }
    return false;
}

void KoPathTool::ActiveParameterHandle::paint( QPainter &painter, KoViewConverter &converter )
{
    painter.save();
    painter.setMatrix( m_parameterShape->transformationMatrix(&converter) * painter.matrix() );

    m_parameterShape->paintHandle( painter, converter, m_handleId, m_tool->m_handleRadius );
    painter.restore();
}

void KoPathTool::ActiveParameterHandle::repaint() const
{
    m_tool->repaint( m_parameterShape->shapeToDocument( QRectF( m_parameterShape->handlePosition( m_handleId ), QSize( 1, 1 ) ) ) );
}

void KoPathTool::ActiveParameterHandle::mousePressEvent( KoPointerEvent *event ) 
{
    if( event->button() & Qt::LeftButton )
    {
        m_tool->m_pointSelection.clear();
        m_tool->m_currentStrategy = new KoParameterChangeStrategy( m_tool, m_tool->m_canvas, m_parameterShape, m_handleId );
    }
}

bool KoPathTool::ActiveParameterHandle::check() 
{
    return m_tool->m_canvas->shapeManager()->selection()->isSelected( m_parameterShape );
}

void KoPathTool::KoPathPointSelection::paint( QPainter &painter, KoViewConverter &converter )
{
    KoPathShapePointMap::iterator it( m_shapePointMap.begin() );
    for ( ; it != m_shapePointMap.end(); ++it )
    {
        painter.save();

        painter.setMatrix( it.key()->transformationMatrix(&converter) * painter.matrix() );
        KoShape::applyConversion( painter, converter );

        QRectF handle = converter.viewToDocument( m_tool->handleRect( QPoint(0,0) ) );

        foreach( KoPathPoint *p, it.value() )
            p->paint( painter, handle.size(), KoPathPoint::All );

        painter.restore();
    }
}

void KoPathTool::KoPathPointSelection::add( KoPathPoint * point, bool clear )
{
    bool allreadyIn = false;
    if ( clear )
    {
        if ( size() == 1 && m_selectedPoints.contains( point ) )
        {
            allreadyIn = true;
        }
        else
        {
            this->clear();
        }
    }
    else
    {
        allreadyIn = m_selectedPoints.contains( point );
    }

    if ( !allreadyIn )
    {
        m_selectedPoints.insert( point );
        KoPathShape * pathShape = point->parent();
        KoPathShapePointMap::iterator it( m_shapePointMap.find( pathShape ) );
        if ( it == m_shapePointMap.end() )
        {
            it = m_shapePointMap.insert( pathShape, QSet<KoPathPoint *>() );
        }
        it.value().insert( point );
        m_tool->repaint( point->boundingRect() );
    }
}

void KoPathTool::KoPathPointSelection::remove( KoPathPoint * point )
{
    if ( m_selectedPoints.remove( point ) )
    {
        KoPathShape * pathShape = point->parent();
        m_shapePointMap[pathShape].remove( point );
        if ( m_shapePointMap[pathShape].size() == 0 )
        {
            m_shapePointMap.remove( pathShape );
        }
    }
    m_tool->repaint( point->boundingRect() );
}

void KoPathTool::KoPathPointSelection::clear()
{
    repaint();
    m_selectedPoints.clear();
    m_shapePointMap.clear();
}

void KoPathTool::KoPathPointSelection::repaint()
{
    foreach ( KoPathPoint *p, m_selectedPoints )
    {
        m_tool->repaint( p->boundingRect() );
    }
}

void KoPathTool::KoPathPointSelection::update()
{
    QSet<KoShape *> shapes = m_tool->m_canvas->shapeManager()->selection()->selectedShapes().toSet();

    KoPathShapePointMap::iterator it( m_shapePointMap.begin() );
    while ( it != m_shapePointMap.end() )
    {
        if ( ! shapes.contains( it.key() ) )
        {
            it = m_shapePointMap.erase( it );
        }
        else
        {
            QSet<KoPathPoint *>::iterator pointIt( it.value().begin() );
            while ( pointIt != it.value().end() )
            {
                if ( ( *pointIt )->parent()->pathPointIndex( *pointIt ) == KoPathPointIndex( -1, -1 ) )
                {
                    pointIt = it.value().erase( pointIt );
                }
                else
                {
                    ++pointIt;
                }
            }
            ++it;
        }
    }
}

QList<KoPathPointData> KoPathTool::KoPathPointSelection::selectedPointsData() const
{
    QList<KoPathPointData> pointData;
    foreach( KoPathPoint* p, m_selectedPoints )
    {
        KoPathShape * pathShape = p->parent();
        pointData.append( KoPathPointData( pathShape, pathShape->pathPointIndex( p ) ) );
    }
    return pointData;
}

QList<KoPathPointData> KoPathTool::KoPathPointSelection::selectedSegmentsData() const
{
    QList<KoPathPointData> pointData;

    QList<KoPathPointData> pd( selectedPointsData() );
    qSort( pd );

    KoPathPointData last( 0, KoPathPointIndex( -1, -1 ) );
    KoPathPointData lastSubpathStart( 0, KoPathPointIndex( -1, -1 ) );

    QList<KoPathPointData>::const_iterator it( pd.begin() );
    for ( ; it != pd.end(); ++it )
    {
        if ( it->m_pointIndex.second == 0 )
            lastSubpathStart = *it;

        if ( last.m_pathShape == it->m_pathShape 
             && last.m_pointIndex.first == it->m_pointIndex.first 
             && last.m_pointIndex.second + 1 == it->m_pointIndex.second  )
        {
            pointData.append( last );
        }

        if ( lastSubpathStart.m_pathShape == it->m_pathShape 
             && it->m_pathShape->pointByIndex( it->m_pointIndex )->properties() & KoPathPoint::CloseSubpath )
        {
            pointData.append( *it );
        }

        last = *it;
    }

    return pointData;
}

#include "KoPathTool.moc"
