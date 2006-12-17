/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoPathCommand.h"
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
    m_pointTypeGroup->addButton( button, Corner );
    layout->addWidget( button, 0, 0 );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathpoint-smooth") );
    button->setToolTip( i18n( "Smooth point" ) );
    m_pointTypeGroup->addButton( button, Smooth );
    layout->addWidget( button, 0, 1 );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("pathpoint-symmetric") );
    button->setToolTip( i18n( "Symmetric point" ) );
    m_pointTypeGroup->addButton( button, Symmetric );
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

void KoPathTool::slotPointTypeChanged( int type ) {
    QList<KoPathPoint*> selectedPoints = m_pointSelection.selectedPoints().toList();
    if( ! selectedPoints.size() )
        return;

    QList<KoPathPoint*> points;
    QList<KoPathPoint::KoPointProperties> properties;

    foreach( KoPathPoint *point, selectedPoints )
    {
        KoPathPoint::KoPointProperties props = point->properties();
        if( (props & KoPathPoint::HasControlPoint1) == 0 || (props & KoPathPoint::HasControlPoint2) == 0 )
            continue;

        points.append( point );
        if( type == Symmetric )
        {
            props &= ~KoPathPoint::IsSmooth;
            props |= KoPathPoint::IsSymmetric;
        }
        else if( type == Smooth )
        {
            props &= ~KoPathPoint::IsSymmetric;
            props |= KoPathPoint::IsSmooth;
        }
        else
        {
            props &= ~KoPathPoint::IsSymmetric;
            props &= ~KoPathPoint::IsSmooth;
        }

        properties.append( props );

    }
    if( points.count() == 0 )
        return;

    KoPointPropertyCommand *cmd = new KoPointPropertyCommand( points, properties );
    m_canvas->addCommand( cmd, true );
}

void KoPathTool::insertPoints() {
    if ( m_pointSelection.objectCount() == 1 )
    {
        QList<KoPathPoint*> selectedPoints = m_pointSelection.selectedPoints().toList();
        KoPathShape * pathShape = selectedPoints[0]->parent();
        QList<KoPathSegment> segments;
        foreach( KoPathPoint* p, selectedPoints )
        {
            KoPathPoint *n = pathShape->nextPoint( p );
            if( m_pointSelection.contains( n ) )
                segments << qMakePair( p, n );
        }
        if( segments.size() )
        {
            KoSegmentSplitCommand *cmd = new KoSegmentSplitCommand( pathShape, segments, 0.5 );
            m_canvas->addCommand( cmd, true );
        }
    }
}

void KoPathTool::removePoints() {
    // TODO finish current action or should this not possible during actions???
    if ( m_pointSelection.size() > 0 )
    {
        KoPointRemoveCommand *cmd = new KoPointRemoveCommand( m_pointSelection.selectedPointMap() );
        ActivePointHandle *pointHandle = dynamic_cast<ActivePointHandle*>( m_activeHandle );
        if ( pointHandle && m_pointSelection.contains( pointHandle->m_activePoint ) )
        {
            delete m_activeHandle;
            m_activeHandle = 0;
        }
        m_pointSelection.clear();
        m_canvas->addCommand( cmd, true );
    }
}

void KoPathTool::segmentToLine() {
    if ( m_pointSelection.objectCount() == 1 )
    {
        QList<KoPathPoint*> selectedPoints = m_pointSelection.selectedPoints().toList();
        KoPathShape * pathShape = selectedPoints[0]->parent();
        QList<KoPathSegment> segments;
        foreach( KoPathPoint* p, selectedPoints )
        {
            KoPathPoint *n = pathShape->nextPoint( p );
            if( m_pointSelection.contains( n ) )
                segments << qMakePair( p, n );
        }
        if( segments.size() )
        {
            KoSegmentTypeCommand *cmd = new KoSegmentTypeCommand( pathShape, segments, true );
            m_canvas->addCommand( cmd, true );
        }
    }
}

void KoPathTool::segmentToCurve() {
    if ( m_pointSelection.objectCount() == 1 )
    {
        QList<KoPathPoint*> selectedPoints = m_pointSelection.selectedPoints().toList();
        KoPathShape * pathShape = selectedPoints[0]->parent();
        QList<KoPathSegment> segments;
        foreach( KoPathPoint* p, selectedPoints )
        {
            KoPathPoint *n = pathShape->nextPoint( p );
            if( m_pointSelection.contains( n ) )
                segments << qMakePair( p, n );
        }
        if( segments.size() )
        {
            KoSegmentTypeCommand *cmd = new KoSegmentTypeCommand( pathShape, segments, false );
            m_canvas->addCommand( cmd, true );
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
        m_canvas->addCommand( new KoParameterToPathCommand( shapesToConvert ), true );
}

void KoPathTool::joinPoints()
{
    if ( m_pointSelection.objectCount() == 1 )
    {
        QList<KoPathPoint*> selectedPoints = m_pointSelection.selectedPoints().toList();
        KoPathShape * pathShape = selectedPoints[0]->parent();
        if( selectedPoints.size() == 2 )
        {
            KoPointJoinCommand *cmd = new KoPointJoinCommand( pathShape, selectedPoints[0], selectedPoints[1] );
            m_canvas->addCommand( cmd, true );
        }
    }
}

void KoPathTool::breakAtPoint()
{
    if ( m_pointSelection.objectCount() == 1 )
    {
        QList<KoPathPoint*> selectedPoints = m_pointSelection.selectedPoints().toList();
        KoPathShape * pathShape = selectedPoints[0]->parent();
        KoSubpathBreakCommand *cmd = new KoSubpathBreakCommand( pathShape, selectedPoints.first() );
        m_canvas->addCommand( cmd, true );
    }
}

void KoPathTool::breakAtSegment()
{
    if ( m_pointSelection.objectCount() == 1 )
    {
        QList<KoPathPoint*> selectedPoints = m_pointSelection.selectedPoints().toList();
        KoPathShape * pathShape = selectedPoints[0]->parent();
        if( selectedPoints.size() >= 2 )
        {
            KoSubpathBreakCommand *cmd = new KoSubpathBreakCommand( pathShape, KoPathSegment( selectedPoints[0], selectedPoints[1] ) );
            m_canvas->addCommand( cmd, true );
        }
    }
}

void KoPathTool::paint( QPainter &painter, KoViewConverter &converter) {
    KoSelectionSet selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );

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
                parameterShape->paintHandles( painter, converter );
            }
            else
            {
                pathShape->paintPoints(  painter, converter );
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

    m_pointSelection.paint( painter, converter );
    
    painter.setBrush( Qt::red ); // TODO make color configurable
    painter.setPen( Qt::blue );

    if ( m_activeHandle )
        m_activeHandle->paint( painter, converter );

    painter.restore();
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
    
    KoSelectionSet selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
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
        KCommand *command = m_currentStrategy->createCommand();
        if ( command )
            m_canvas->addCommand( command, false );
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
                break;
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
                if(event->modifiers() & Qt::ControlModifier) 
                {
                    if( m_handleRadius > 3 )
                        m_handleRadius--;
                }
                else
                    m_handleRadius++;
                m_pointSelection.repaint();
                break;
            case Qt::Key_Backspace:
                removePoints();
                break;
            case Qt::Key_Insert:
                insertPoints();
                break;
            case Qt::Key_D:
                if ( pathShape )
                {
                    if ( pathShape )
                    {
                        pathShape->debugPath();
                    }
                }
                break;
            case Qt::Key_B:
                if( m_pointSelection.selectedPoints().size() == 1 )
                    breakAtPoint();
                else if( m_pointSelection.selectedPoints().size() >= 2 )
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
    foreach(KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes()) 
    {
        KoPathShape *pathShape;
        pathShape = dynamic_cast<KoPathShape*> (shape);
        if( pathShape )
        {
            found = true;
            repaint( pathShape->shapeToDocument( pathShape->outline().controlPointRect() ) );
        }
    }
    if( !found ) {
        emit sigDone();
        return;
    }
    useCursor(Qt::ArrowCursor, true);
}

void KoPathTool::deactivate() {
    qDebug() << "KoPathTool::deactivate()";
    m_pointSelection.clear();
    delete m_activeHandle;
    m_activeHandle = 0;
    delete m_currentStrategy;
    m_currentStrategy = 0;
}

void KoPathTool::selectPoints( const QRectF &rect, bool clearSelection )
{
    if( clearSelection )
    {
        m_pointSelection.clear();
    }

    KoSelectionSet selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
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
    //qDebug() << "KoPathTool::repaint(" << repaintRect << ")";
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
            m_tool->m_currentStrategy = new KoPathControlPointMoveStrategy( m_tool, m_tool->m_canvas, m_activePoint, m_activePointType, event->point );
        }
            
    }
    else if( event->button() & Qt::RightButton )
    {
        KoPathPoint::KoPointProperties props = m_activePoint->properties();
        if( (props & KoPathPoint::HasControlPoint1) == 0 || (props & KoPathPoint::HasControlPoint2) == 0 )
            return;

        // cycle the smooth->symmetric->unsmooth state of the path point
        if( props & KoPathPoint::IsSmooth )
        {
            props &= ~KoPathPoint::IsSmooth;
            props |= KoPathPoint::IsSymmetric;
        }
        else if( props & KoPathPoint::IsSymmetric )
            props &= ~KoPathPoint::IsSymmetric;
        else
            props |= KoPathPoint::IsSmooth;

        KoPointPropertyCommand *cmd = new KoPointPropertyCommand( m_activePoint, props );
        m_tool->m_canvas->addCommand( cmd, true );
    }
}

void KoPathTool::ActiveParameterHandle::paint( QPainter &painter, KoViewConverter &converter )
{ 
    painter.save();
    painter.setMatrix( m_parameterShape->transformationMatrix(&converter) * painter.matrix() );

    QRectF handle = converter.viewToDocument( m_tool->handleRect( QPoint(0,0) ) );
    m_parameterShape->paintHandle( painter, converter, m_handleId );
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

#include "KoPathTool.moc"
