/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 *               2007 Thomas Zander <zander@kde.org>
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
#include "KoShapeManager.h"
#include "KoCanvasResourceProvider.h"
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
#include "KoPathControlPointMoveStrategy.h"
#include "KoParameterChangeStrategy.h"
#include "KoPathPointMoveStrategy.h"
#include "KoPathPointRubberSelectStrategy.h"
#include "PathToolOptionWidget.h"

#include <KIcon>
#include <kdebug.h>
#include <klocale.h>
#include <QPainter>
#include <QAction>

KoPathTool::KoPathTool(KoCanvasBase *canvas)
: KoTool(canvas)
, m_activeHandle( 0 )
, m_handleRadius( 3 )
, m_pointSelection( this )
, m_currentStrategy(0)
{
    QActionGroup *points = new QActionGroup(this);
    // m_pointTypeGroup->setExclusive( true );
    m_actionPathPointCorner = new QAction(KIcon("pathpoint-corner"), i18n("Corner point"), this);
    addAction("pathpoint-corner", m_actionPathPointCorner);
    m_actionPathPointCorner->setData(KoPathPointTypeCommand::Corner );
    points->addAction(m_actionPathPointCorner);

    m_actionPathPointSmooth = new QAction(KIcon("pathpoint-smooth"), i18n("Smooth point"), this);
    addAction("pathpoint-smooth", m_actionPathPointSmooth);
    m_actionPathPointSmooth->setData(KoPathPointTypeCommand::Smooth );
    points->addAction(m_actionPathPointSmooth);

    m_actionPathPointSymmetric = new QAction(KIcon("pathpoint-symmetric"), i18n("Symmetric Point"), this);
    addAction("pathpoint-symmetric", m_actionPathPointSymmetric );
    m_actionPathPointSymmetric->setData(KoPathPointTypeCommand::Symmetric );
    points->addAction(m_actionPathPointSymmetric);

    m_actionLineSegment = new QAction(KIcon("pathsegment-line"), i18n("Segment to Line"), this);
    m_actionLineSegment->setShortcut(Qt::Key_F);
    addAction("pathsegment-line", m_actionLineSegment );
    connect( m_actionLineSegment, SIGNAL( triggered() ), this, SLOT( segmentToLine() ) );

    m_actionCurveSegment = new QAction(KIcon("pathsegment-curve"), i18n("Segment to Curve"), this);
    m_actionCurveSegment->setShortcut(Qt::Key_C);
    addAction("pathsegment-curve", m_actionCurveSegment );
    connect( m_actionCurveSegment, SIGNAL( triggered() ), this, SLOT( segmentToCurve() ) );

    m_actionAddPoint = new QAction(KIcon("pathpoint-insert"), i18n("Insert point"), this);
    addAction("pathpoint-insert", m_actionAddPoint );
    m_actionAddPoint->setShortcut(Qt::Key_Insert);
    connect( m_actionAddPoint, SIGNAL( triggered() ), this, SLOT( insertPoints() ) );

    m_actionRemovePoint = new QAction(KIcon("pathpoint-remove"), i18n("Remove point"), this);
    m_actionRemovePoint->setShortcut(Qt::Key_Backspace);
    addAction("pathpoint-remove", m_actionRemovePoint );
    connect( m_actionRemovePoint, SIGNAL( triggered() ), this, SLOT( removePoints() ) );

    m_actionBreakPoint = new QAction(KIcon("path-break-point"), i18n("Break at point"), this);
    addAction("path-break-point", m_actionBreakPoint );
    connect( m_actionBreakPoint, SIGNAL( triggered() ), this, SLOT( breakAtPoint() ) );

    m_actionBreakSegment = new QAction(KIcon("path-break-segment"), i18n("Break at segment"), this);
    addAction("path-break-segment", m_actionBreakSegment );
    connect( m_actionBreakSegment, SIGNAL( triggered() ), this, SLOT( breakAtSegment() ) );

    m_actionJoinSegment = new QAction(KIcon("pathpoint-join"), i18n("Join with segment"), this);
    m_actionJoinSegment->setShortcut(Qt::Key_J);
    addAction("pathpoint-join", m_actionJoinSegment );
    connect( m_actionJoinSegment, SIGNAL( triggered() ), this, SLOT( joinPoints() ) );

    m_actionMergePoints = new QAction(KIcon("pathpoint-merge"), i18n("Merge points"), this);
    m_actionMergePoints->setEnabled(false); // not implemented yet.
    addAction("pathpoint-merge", m_actionMergePoints );

    m_actionConvertToPath = new QAction(KIcon("convert-to-path"), i18n("To Path"), this);
    m_actionConvertToPath->setShortcut(Qt::Key_P);
    addAction("convert-to-path", m_actionConvertToPath );
    connect( m_actionConvertToPath, SIGNAL( triggered() ), this, SLOT( convertToPath() ) );

    connect( points, SIGNAL(triggered(QAction*)), this, SLOT( pointTypeChanged( QAction* ) ) );
}

KoPathTool::~KoPathTool() {
}

QWidget * KoPathTool::createOptionWidget() {
    PathToolOptionWidget *widget = new PathToolOptionWidget(this);
    connect(this, SIGNAL(typeChanged(int)), widget, SLOT(setSelectionType(int)));
    connect(this, SIGNAL(pathChanged(KoPathShape*)), widget, SLOT(setSelectedPath(KoPathShape*)));
    updateOptionsWidget();
    return widget;
}

void KoPathTool::pointTypeChanged( QAction *type )
{
    if ( !m_pointSelection.isEmpty() )
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

        if ( !pointToChange.isEmpty() )
        {
            KoPathPointTypeCommand *cmd = new KoPathPointTypeCommand( pointToChange,
                    static_cast<KoPathPointTypeCommand::PointType> (type->data().toInt()));
            m_canvas->addCommand( cmd );
            updateActions();
        }
    }
}

void KoPathTool::insertPoints() 
{
    if ( m_pointSelection.size() > 1 )
    {
        QList<KoPathPointData> segments( m_pointSelection.selectedSegmentsData() );
        if ( !segments.isEmpty() )
        {
            KoPathPointInsertCommand *cmd = new KoPathPointInsertCommand( segments, 0.5 );
            m_canvas->addCommand( cmd );
            updateActions();
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
        updateActions();
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
            updateActions();
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
            updateActions();
        }
    }
}

void KoPathTool::convertToPath()
{
    QList<KoParameterShape*> shapesToConvert;
    foreach( KoShape *shape, m_selectedShapes)
    {
        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( shape );
        if ( parameterShape && parameterShape->isParametricShape() )
            shapesToConvert.append( parameterShape );
    }
    if( shapesToConvert.count() )
        m_canvas->addCommand( new KoParameterToPathCommand( shapesToConvert ) );
    updateOptionsWidget();
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
        updateActions();
    }
}

void KoPathTool::breakAtPoint()
{
    if ( !m_pointSelection.isEmpty() )
    {
        m_canvas->addCommand( new KoPathBreakAtPointCommand( m_pointSelection.selectedPointsData() ) );
        updateActions();
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
            updateActions();
        }
    }
}

void KoPathTool::paint( QPainter &painter, const KoViewConverter &converter) {
    painter.setRenderHint( QPainter::Antialiasing, true );
    // use different colors so that it is also visible on a background of the same color
    painter.setBrush( Qt::white ); //TODO make configurable
    painter.setPen( Qt::blue );

    foreach( KoPathShape *shape, m_selectedShapes )
    {
        painter.save();
        painter.setMatrix( shape->absoluteTransformation( &converter ) * painter.matrix() );

        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( shape );
        if ( parameterShape && parameterShape->isParametricShape() )
        {
            parameterShape->paintHandles( painter, converter, m_handleRadius );
        }
        else
        {
            shape->paintPoints( painter, converter, m_handleRadius );
        }

        painter.restore();
    }

    if ( m_currentStrategy )
    {
        painter.save();
        m_currentStrategy->paint( painter, converter );
        painter.restore();
    }

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
}

void KoPathTool::repaintDecorations()
{
    foreach(KoShape *shape, m_selectedShapes)
    {
        repaint( shape->boundingRect() );
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
            if( (event->modifiers() & Qt::ControlModifier) == 0 )
            {
                m_pointSelection.clear();
                updateActions();
            }
            // start rubberband selection
            Q_ASSERT(m_currentStrategy == 0);
            m_currentStrategy = new KoPathPointRubberSelectStrategy( this, m_canvas, event->point );
        }
    }
}

void KoPathTool::mouseMoveEvent( KoPointerEvent *event ) {
    if( event->button() & Qt::RightButton )
        return;

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

    foreach( KoPathShape *shape, m_selectedShapes )
    {
        QRectF roi = handleRect( shape->documentToShape( event->point ) );
        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( shape );
        if ( parameterShape && parameterShape->isParametricShape() )
        {
            int handleId = parameterShape->handleIdAt( roi );
            //qDebug() << "handleId" << handleId;
            if ( handleId != -1 )
            {
                useCursor(Qt::SizeAllCursor);
                if(m_activeHandle)
                    m_activeHandle->repaint();
                delete m_activeHandle;
                m_activeHandle = new ActiveParameterHandle( this, parameterShape, handleId );
                m_activeHandle->repaint();
                return;
            }

        }
        else
        {
            QList<KoPathPoint*> points = shape->pointsAt( roi );
            if( ! points.empty() )
            {
                KoPathPoint *p = points.first();
                KoPathPoint::KoPointType type = KoPathPoint::Node;

                // the node point must be hit if the point is not selected yet
                if( ! m_pointSelection.contains( p ) && ! roi.contains( p->point() ) )
                    return;

                // check for the control points as otherwise it is no longer 
                // possible to change the control points when they are the same as the point
                if( p->properties() & KoPathPoint::HasControlPoint1 && roi.contains( p->controlPoint1() ) )
                    type = KoPathPoint::ControlPoint1;
                else if( p->properties() & KoPathPoint::HasControlPoint2 && roi.contains( p->controlPoint2() ) )
                    type = KoPathPoint::ControlPoint2;

                useCursor(Qt::SizeAllCursor);

                ActivePointHandle *prev = dynamic_cast<ActivePointHandle*> (m_activeHandle);
                if(prev && prev->m_activePoint == p && prev->m_activePointType == type)
                    return; // no change;

                if(m_activeHandle)
                    m_activeHandle->repaint();
                delete m_activeHandle;
                m_activeHandle = new ActivePointHandle( this, p, type );
                m_activeHandle->repaint();
                return;
            }
        }
    }

    useCursor(Qt::ArrowCursor);
    if(m_activeHandle)
        m_activeHandle->repaint();
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

        if( m_selectedShapes.count() == 1 )
            emit pathChanged( m_selectedShapes.first() );
        else
            emit pathChanged( 0 );
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
// TODO move these to the actions in the constructor.
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
    // retrieve the actual global handle radius
    m_handleRadius = m_canvas->resourceProvider()->handleRadius();

    repaintDecorations();
    m_selectedShapes.clear();
    foreach(KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes())
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*> (shape);

        if ( !shape->isLocked() && pathShape )
        {
            // as the tool is just in activation repaintDecorations does not yet get called
            // so we need to use repaint of the tool and it is only needed to repaint the 
            // current canvas
            repaint( pathShape->boundingRect() );
            m_selectedShapes.append(pathShape);
        }
    }
    if( m_selectedShapes.isEmpty() ) {
        emit done();
        return;
    }
    useCursor(Qt::ArrowCursor, true);
    connect(m_canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SLOT(activate()));
    updateOptionsWidget();
    updateActions();
}

void KoPathTool::updateOptionsWidget() {
    PathToolOptionWidget::Types type;
    foreach(KoPathShape *shape, m_selectedShapes)
    {
        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( shape );
        type |= parameterShape && parameterShape->isParametricShape() ?
            PathToolOptionWidget::ParametricShape : PathToolOptionWidget::PlainPath;
    }
    if( m_selectedShapes.count() == 1 )
        emit pathChanged( m_selectedShapes.first() );
    else 
        emit pathChanged( 0 );
    emit typeChanged(type);
}

void KoPathTool::updateActions() {
    const bool hasPointsSelected = !m_pointSelection.isEmpty();
    m_actionPathPointCorner->setEnabled(hasPointsSelected);
    m_actionPathPointSmooth->setEnabled(hasPointsSelected);
    m_actionPathPointSymmetric->setEnabled(hasPointsSelected);
    m_actionRemovePoint->setEnabled(hasPointsSelected);
    m_actionBreakPoint->setEnabled(hasPointsSelected);

    bool hasSegmentsSelected = false;
    if ( hasPointsSelected && m_pointSelection.size() > 1 )
        hasSegmentsSelected = !m_pointSelection.selectedSegmentsData().isEmpty();
    m_actionAddPoint->setEnabled(hasSegmentsSelected);
    m_actionLineSegment->setEnabled(hasSegmentsSelected);
    m_actionCurveSegment->setEnabled(hasSegmentsSelected);
    m_actionBreakSegment->setEnabled(hasPointsSelected && m_pointSelection.objectCount() == 1 && m_pointSelection.size() == 2);
    m_actionJoinSegment->setEnabled(hasPointsSelected && m_pointSelection.objectCount() == 1 && m_pointSelection.size() == 2);
    //m_actionMergePoints->setEnabled(false);
}

void KoPathTool::deactivate() {
    disconnect(m_canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SLOT(activate()));
    m_pointSelection.clear();
    m_selectedShapes.clear();

    delete m_activeHandle;
    m_activeHandle = 0;
    delete m_currentStrategy;
    m_currentStrategy = 0;
}

void KoPathTool::resourceChanged( int key, const QVariant & res )
{
    if(key != KoCanvasResource::HandleRadius)
        return;

    int oldHandleRadius = m_handleRadius;

    m_handleRadius = res.toUInt();

    // repaint with the bigger of old and new handle radius
    int maxRadius = qMax( m_handleRadius, oldHandleRadius );
    foreach(KoPathShape *shape, m_selectedShapes)
    {
        QRectF controlPointRect = shape->absoluteTransformation(0).map( shape->outline() ).controlPointRect();
        repaint( controlPointRect.adjusted(-maxRadius,-maxRadius,maxRadius,maxRadius) );
    }
}

void KoPathTool::selectPoints( const QRectF &rect, bool clearSelection )
{
    if( clearSelection )
    {
        m_pointSelection.clear();
    }

    foreach(KoPathShape* shape, m_selectedShapes)
    {
        KoParameterShape *parameterShape = dynamic_cast<KoParameterShape*>( shape );
        if(parameterShape && parameterShape->isParametricShape() )
            continue;
        foreach( KoPathPoint* point, shape->pointsAt( shape->documentToShape( rect ) ))
            m_pointSelection.add( point, false );
    }
    updateActions();
}

void KoPathTool::repaint( const QRectF &repaintRect ) {
    //kDebug(30006) <<"KoPathTool::repaint(" << repaintRect <<")" << m_handleRadius;
    m_canvas->updateCanvas( repaintRect.adjusted( -m_handleRadius, -m_handleRadius, m_handleRadius, m_handleRadius ) );
}

QRectF KoPathTool::handleRect( const QPointF &p ) {
    return QRectF( p.x()-m_handleRadius, p.y()-m_handleRadius, 2*m_handleRadius, 2*m_handleRadius );
}

void KoPathTool::ActivePointHandle::paint( QPainter &painter, const KoViewConverter &converter )
{ 
    painter.save();
    painter.setMatrix( m_activePoint->parent()->absoluteTransformation(&converter) * painter.matrix() );
    KoShape::applyConversion( painter, converter );

    QRectF handle = converter.viewToDocument( m_tool->handleRect( QPoint(0,0) ) );
    m_activePoint->paint( painter, handle.size(), m_tool->m_pointSelection.contains( m_activePoint ) ? KoPathPoint::All : KoPathPoint::Node );
    painter.restore();
}

void KoPathTool::ActivePointHandle::repaint() const
{
    m_tool->repaint( m_activePoint->boundingRect( !m_tool->m_pointSelection.contains(m_activePoint) ) );
}

void KoPathTool::ActivePointHandle::mousePressEvent( KoPointerEvent *event ) 
{
    if( ( event->button() & Qt::LeftButton ) == 0)
        return;
    if((event->modifiers() & Qt::ShiftButton) == 0) { // no shift pressed.
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
            m_tool->repaint( m_activePoint->boundingRect(false) );
        }
        else
        {
            // no control modifier, so clear selection and select active point
            if ( !m_tool->m_pointSelection.contains( m_activePoint ) )
            {
                m_tool->m_pointSelection.add( m_activePoint, true );
                m_tool->repaint( m_activePoint->boundingRect(false) );
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
    else {
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
    m_tool->updateActions();
}

bool KoPathTool::ActivePointHandle::check() 
{
    if ( m_tool->m_canvas->shapeManager()->selection()->isSelected( m_activePoint->parent() ) )
    {
        return m_activePoint->parent()->pathPointIndex( m_activePoint ) != KoPathPointIndex( -1, -1 );
    }
    return false;
}

void KoPathTool::ActiveParameterHandle::paint( QPainter &painter, const KoViewConverter &converter )
{
    painter.save();
    painter.setMatrix( m_parameterShape->absoluteTransformation(&converter) * painter.matrix() );

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
        m_tool->updateActions();
    }
}

bool KoPathTool::ActiveParameterHandle::check() 
{
    return m_tool->m_canvas->shapeManager()->selection()->isSelected( m_parameterShape );
}

void KoPathTool::KoPathPointSelection::paint( QPainter &painter, const KoViewConverter &converter )
{
    KoPathShapePointMap::iterator it( m_shapePointMap.begin() );
    for ( ; it != m_shapePointMap.end(); ++it )
    {
        painter.save();

        painter.setMatrix( it.key()->absoluteTransformation(&converter) * painter.matrix() );
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
        m_tool->repaint( p->boundingRect(false) );
    }
}

void KoPathTool::KoPathPointSelection::update()
{
    KoPathShapePointMap::iterator it( m_shapePointMap.begin() );
    while ( it != m_shapePointMap.end() )
    {
        if ( ! m_tool->m_selectedShapes.contains( it.key() ) )
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
