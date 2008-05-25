/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 *               2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#include "KoPathToolHandle.h"
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
#include <KoPathPoint.h>
#include "KoPathPointRubberSelectStrategy.h"
#include "PathToolOptionWidget.h"
#include "KoConnectionShape.h"
#include "KoSnapGuide.h"
#include "SnapGuideConfigWidget.h"

#include <KIcon>
#include <kdebug.h>
#include <klocale.h>
#include <QtGui/QPainter>
#include <QtGui/QAction>
#include <QtGui/QBitmap>
#include <QtGui/QTabWidget>

static unsigned char needle_bits[] = {
    0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x60, 0x00, 0xc0, 0x00, 0xc0, 0x01,
    0x80, 0x03, 0x80, 0x07, 0x00, 0x0f, 0x00, 0x1f, 0x00, 0x3e, 0x00, 0x7e,
    0x00, 0x7c, 0x00, 0x1c, 0x00, 0x18, 0x00, 0x00};

static unsigned char needle_move_bits[] = {
    0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x60, 0x00, 0xc0, 0x00, 0xc0, 0x01,
    0x80, 0x03, 0x80, 0x07, 0x10, 0x0f, 0x38, 0x1f, 0x54, 0x3e, 0xfe, 0x7e,
    0x54, 0x7c, 0x38, 0x1c, 0x10, 0x18, 0x00, 0x00};


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
    connect( &m_pointSelection, SIGNAL(selectionChanged()), this, SLOT(pointSelectionChanged()) );

    QBitmap b = QBitmap::fromData( QSize(16, 16), needle_bits );
    QBitmap m = b.createHeuristicMask( false );

    m_selectCursor = QCursor( b, m, 2, 0 );

    b = QBitmap::fromData( QSize(16, 16), needle_move_bits );
    m = b.createHeuristicMask( false );

    m_moveCursor = QCursor( b, m, 2, 0 );
}

KoPathTool::~KoPathTool() {
}

QWidget * KoPathTool::createOptionWidget() 
{
    QTabWidget * widget = new QTabWidget(0);

    PathToolOptionWidget * toolOptions = new PathToolOptionWidget(this);
    connect(this, SIGNAL(typeChanged(int)), toolOptions, SLOT(setSelectionType(int)));
    //connect(this, SIGNAL(pathChanged(KoPathShape*)), widget, SLOT(setSelectedPath(KoPathShape*)));
    updateOptionsWidget();

    SnapGuideConfigWidget * snapOptions = new SnapGuideConfigWidget( m_canvas->snapGuide(), widget );
    widget->addTab( toolOptions, i18n("Default") );
    widget->addTab( snapOptions, i18n("Snap Guides") );

    return widget;
}

void KoPathTool::pointTypeChanged( QAction *type )
{
    if ( !m_pointSelection.hasSelection() )
    {
        QList<KoPathPointData> selectedPoints = m_pointSelection.selectedPointsData();
        QList<KoPathPointData> pointToChange;

        QList<KoPathPointData>::const_iterator it( selectedPoints.begin() );
        for ( ; it != selectedPoints.end(); ++it )
        {
            KoPathPoint *point = it->pathShape->pointByIndex( it->pointIndex );
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
        PointHandle *pointHandle = dynamic_cast<PointHandle*>( m_activeHandle );
        if ( pointHandle && m_pointSelection.contains( pointHandle->activePoint() ) )
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
    foreach( KoShape *shape, m_pointSelection.selectedShapes() )
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
        KoPathShape * pathShape = pd1.pathShape;
        if ( !pathShape->isClosedSubpath( pd1.pointIndex.first ) &&
            ( pd1.pointIndex.second == 0 ||
              pd1.pointIndex.second == pathShape->pointCountSubpath( pd1.pointIndex.first ) - 1 ) &&
             !pathShape->isClosedSubpath( pd2.pointIndex.first ) &&
            ( pd2.pointIndex.second == 0 ||
              pd2.pointIndex.second == pathShape->pointCountSubpath( pd2.pointIndex.first ) - 1 ) )
        {
            KoSubpathJoinCommand *cmd = new KoSubpathJoinCommand( pd1, pd2 );
            m_canvas->addCommand( cmd );
        }
        updateActions();
    }
}

void KoPathTool::breakAtPoint()
{
    if ( !m_pointSelection.hasSelection() )
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

    foreach( KoPathShape *shape, m_pointSelection.selectedShapes() )
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

    if( m_currentStrategy )
    {
        painter.save();
        KoShape::applyConversion( painter, converter );
        m_canvas->snapGuide()->paint( painter, converter );
        painter.restore();
    }
}

void KoPathTool::repaintDecorations()
{
    foreach(KoShape *shape, m_pointSelection.selectedShapes() )
    {
        repaint( shape->boundingRect() );
    }

    m_pointSelection.repaint();
}

void KoPathTool::mousePressEvent( KoPointerEvent *event ) {
    // we are moving if we hit a point and use the left mouse button
    event->ignore();
    if ( m_activeHandle )
    {
        m_currentStrategy = m_activeHandle->handleMousePress( event );
        event->accept();
    }
    else
    {
        if( event->button() & Qt::LeftButton )
        {
            if( (event->modifiers() & Qt::ControlModifier) == 0 )
            {
                m_pointSelection.clear();
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

    foreach( KoPathShape *shape, m_pointSelection.selectedShapes() )
    {
        QRectF roi = handleRect( shape->documentToShape( event->point ) );
        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( shape );
        if ( parameterShape && parameterShape->isParametricShape() )
        {
            int handleId = parameterShape->handleIdAt( roi );
            if ( handleId != -1 )
            {
                useCursor(m_moveCursor);
                if(m_activeHandle)
                    m_activeHandle->repaint();
                delete m_activeHandle;

                if ( KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>( parameterShape ) ) {
                    //qDebug() << "handleId" << handleId;
                    m_activeHandle = new ConnectionHandle( this, connectionShape, handleId );
                    m_activeHandle->repaint();
                    return;
                }
                else {
                    //qDebug() << "handleId" << handleId;
                    m_activeHandle = new ParameterHandle( this, parameterShape, handleId );
                    m_activeHandle->repaint();
                    return;
                }
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
                if( p->activeControlPoint1() && roi.contains( p->controlPoint1() ) )
                    type = KoPathPoint::ControlPoint1;
                else if( p->activeControlPoint2() && roi.contains( p->controlPoint2() ) )
                    type = KoPathPoint::ControlPoint2;

                useCursor( m_moveCursor );

                PointHandle *prev = dynamic_cast<PointHandle*> (m_activeHandle);
                if(prev && prev->activePoint() == p && prev->activePointType() == type)
                    return; // no change;

                if(m_activeHandle)
                    m_activeHandle->repaint();
                delete m_activeHandle;
                m_activeHandle = new PointHandle( this, p, type );
                m_activeHandle->repaint();
                return;
            }
        }
    }

    useCursor( m_selectCursor );
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

        if( m_pointSelection.selectedShapes().count() == 1 )
            emit pathChanged( m_pointSelection.selectedShapes().first() );
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
    m_canvas->snapGuide()->reset();

    repaintDecorations();
    QList<KoPathShape*> selectedShapes;
    foreach(KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes())
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*> (shape);

        if ( !shape->isLocked() && pathShape )
        {
            // as the tool is just in activation repaintDecorations does not yet get called
            // so we need to use repaint of the tool and it is only needed to repaint the
            // current canvas
            repaint( pathShape->boundingRect() );
            selectedShapes.append(pathShape);
        }
    }
    if( selectedShapes.isEmpty() ) {
        emit done();
        return;
    }
    m_pointSelection.setSelectedShapes( selectedShapes );
    useCursor( m_selectCursor, true );
    connect(m_canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SLOT(activate()));
    updateOptionsWidget();
    updateActions();
}

void KoPathTool::updateOptionsWidget() {
    PathToolOptionWidget::Types type;
    QList<KoPathShape*> selectedShapes = m_pointSelection.selectedShapes();
    foreach(KoPathShape *shape, selectedShapes )
    {
        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( shape );
        type |= parameterShape && parameterShape->isParametricShape() ?
            PathToolOptionWidget::ParametricShape : PathToolOptionWidget::PlainPath;
    }
    if( selectedShapes.count() == 1 )
        emit pathChanged( selectedShapes.first() );
    else
        emit pathChanged( 0 );
    emit typeChanged(type);
}

void KoPathTool::updateActions() {
    const bool hasPointsSelected = !m_pointSelection.hasSelection();
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
    m_pointSelection.setSelectedShapes( QList<KoPathShape*>() );
    delete m_activeHandle;
    m_activeHandle = 0;
    delete m_currentStrategy;
    m_currentStrategy = 0;
    m_canvas->snapGuide()->reset();
}

void KoPathTool::resourceChanged( int key, const QVariant & res )
{
    if(key != KoCanvasResource::HandleRadius)
        return;

    int oldHandleRadius = m_handleRadius;

    m_handleRadius = res.toUInt();

    // repaint with the bigger of old and new handle radius
    int maxRadius = qMax( m_handleRadius, oldHandleRadius );
    foreach(KoPathShape *shape, m_pointSelection.selectedShapes() )
    {
        QRectF controlPointRect = shape->absoluteTransformation(0).map( shape->outline() ).controlPointRect();
        repaint( controlPointRect.adjusted(-maxRadius,-maxRadius,maxRadius,maxRadius) );
    }
}

void KoPathTool::pointSelectionChanged()
{
    updateActions();
    m_canvas->snapGuide()->setIgnoredPathPoints( m_pointSelection.selectedPoints().toList() );
}

void KoPathTool::repaint( const QRectF &repaintRect ) {
    //kDebug(30006) <<"KoPathTool::repaint(" << repaintRect <<")" << m_handleRadius;
    // widen border to take antialiasing into account
    double radius = m_handleRadius+1;
    m_canvas->updateCanvas( repaintRect.adjusted( -radius, -radius, radius, radius ) );
}

QRectF KoPathTool::handleRect( const QPointF &p )
{
    QSizeF hsize = m_canvas->viewConverter()->viewToDocument( QSizeF(m_handleRadius,m_handleRadius) );
    return QRectF( p.x()-hsize.width(), p.y()-hsize.height(), 2*hsize.width(), 2*hsize.height() );
}

void KoPathTool::deleteSelection()
{
    removePoints();
}

KoToolSelection * KoPathTool::selection()
{
    return &m_pointSelection;
}

#include "KoPathTool.moc"
