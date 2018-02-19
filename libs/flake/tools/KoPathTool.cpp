/* This file is part of the KDE project
 * Copyright (C) 2006-2012 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
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
#include "KoToolBase_p.h"
#include "KoPathShape_p.h"
#include "KoPathToolHandle.h"
#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoSelectedShapesProxy.h"
#include "KoDocumentResourceManager.h"
#include "KoViewConverter.h"
#include "KoSelection.h"
#include "KoPointerEvent.h"
#include "commands/KoPathPointTypeCommand.h"
#include "commands/KoPathPointInsertCommand.h"
#include "commands/KoPathPointRemoveCommand.h"
#include "commands/KoPathSegmentTypeCommand.h"
#include "commands/KoPathBreakAtPointCommand.h"
#include "commands/KoPathSegmentBreakCommand.h"
#include "commands/KoParameterToPathCommand.h"
#include "commands/KoSubpathJoinCommand.h"
#include <commands/KoMultiPathPointMergeCommand.h>
#include <commands/KoMultiPathPointJoinCommand.h>
#include "KoParameterShape.h"
#include "KoPathPoint.h"
#include "KoPathPointRubberSelectStrategy.h"
#include "KoPathSegmentChangeStrategy.h"
#include "KoPathConnectionPointStrategy.h"
#include "KoParameterChangeStrategy.h"
#include "PathToolOptionWidget.h"
#include "KoConnectionShape.h"
#include "KoSnapGuide.h"
#include "KoShapeController.h"
#include "kis_action_registry.h"
#include <KoShapeStrokeModel.h>
#include "kis_command_utils.h"
#include <KoShapeHandlesCollection.h>
#include <KoCanvasUpdatesCollector.h>

#include "kis_algebra_2d.h"
#include "kis_painting_tweaks.h"

#include <KoIcon.h>

#include <QMenu>
#include <QAction>
#include <FlakeDebug.h>
#include <klocalizedstring.h>
#include <QPainter>
#include <QBitmap>
#include <QTabWidget>

#include <math.h>

static const unsigned char needle_bits[] = {
    0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x60, 0x00, 0xc0, 0x00, 0xc0, 0x01,
    0x80, 0x03, 0x80, 0x07, 0x00, 0x0f, 0x00, 0x1f, 0x00, 0x3e, 0x00, 0x7e,
    0x00, 0x7c, 0x00, 0x1c, 0x00, 0x18, 0x00, 0x00
};

static const unsigned char needle_move_bits[] = {
    0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x60, 0x00, 0xc0, 0x00, 0xc0, 0x01,
    0x80, 0x03, 0x80, 0x07, 0x10, 0x0f, 0x38, 0x1f, 0x54, 0x3e, 0xfe, 0x7e,
    0x54, 0x7c, 0x38, 0x1c, 0x10, 0x18, 0x00, 0x00
};

// helper function to calculate the squared distance between two points
qreal squaredDistance(const QPointF& p1, const QPointF &p2)
{
    qreal dx = p1.x()-p2.x();
    qreal dy = p1.y()-p2.y();
    return dx*dx + dy*dy;
}

struct KoPathTool::PathSegment {
    PathSegment()
        : path(0), segmentStart(0), positionOnSegment(0)
    {
    }

    bool isValid() {
        return  path && segmentStart;
    }

    KoPathShape *path;
    KoPathPoint *segmentStart;
    qreal positionOnSegment;
};

KoPathTool::KoPathTool(KoCanvasBase *canvas)
        : KoToolBase(canvas)
        , m_pointSelection(this)
        , m_activeHandle(0)
        , m_handleRadius(3)
        , m_activeSegment(0)
        , m_currentStrategy(0)
        , m_activatedTemporarily(false)
{
    QActionGroup *points = new QActionGroup(this);
    // m_pointTypeGroup->setExclusive(true);
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    m_actionPathPointCorner = actionRegistry->makeQAction("pathpoint-corner", this);
    addAction("pathpoint-corner", m_actionPathPointCorner);
    m_actionPathPointCorner->setData(KoPathPointTypeCommand::Corner);
    points->addAction(m_actionPathPointCorner);

    m_actionPathPointSmooth = actionRegistry->makeQAction("pathpoint-smooth", this);
    addAction("pathpoint-smooth", m_actionPathPointSmooth);
    m_actionPathPointSmooth->setData(KoPathPointTypeCommand::Smooth);
    points->addAction(m_actionPathPointSmooth);

    m_actionPathPointSymmetric = actionRegistry->makeQAction("pathpoint-symmetric", this);
    addAction("pathpoint-symmetric", m_actionPathPointSymmetric);
    m_actionPathPointSymmetric->setData(KoPathPointTypeCommand::Symmetric);
    points->addAction(m_actionPathPointSymmetric);

    m_actionCurvePoint = actionRegistry->makeQAction("pathpoint-curve", this);
    addAction("pathpoint-curve", m_actionCurvePoint);
    connect(m_actionCurvePoint, SIGNAL(triggered()), this, SLOT(pointToCurve()));

    m_actionLinePoint = actionRegistry->makeQAction("pathpoint-line", this);
    addAction("pathpoint-line", m_actionLinePoint);
    connect(m_actionLinePoint, SIGNAL(triggered()), this, SLOT(pointToLine()));

    m_actionLineSegment = actionRegistry->makeQAction("pathsegment-line", this);
    addAction("pathsegment-line", m_actionLineSegment);
    connect(m_actionLineSegment, SIGNAL(triggered()), this, SLOT(segmentToLine()));

    m_actionCurveSegment = actionRegistry->makeQAction("pathsegment-curve", this);
    addAction("pathsegment-curve", m_actionCurveSegment);
    connect(m_actionCurveSegment, SIGNAL(triggered()), this, SLOT(segmentToCurve()));

    m_actionAddPoint = actionRegistry->makeQAction("pathpoint-insert", this);
    addAction("pathpoint-insert", m_actionAddPoint);
    connect(m_actionAddPoint, SIGNAL(triggered()), this, SLOT(insertPoints()));

    m_actionRemovePoint = actionRegistry->makeQAction("pathpoint-remove", this);
    addAction("pathpoint-remove", m_actionRemovePoint);
    connect(m_actionRemovePoint, SIGNAL(triggered()), this, SLOT(removePoints()));

    m_actionBreakPoint = actionRegistry->makeQAction("path-break-point", this);
    addAction("path-break-point", m_actionBreakPoint);
    connect(m_actionBreakPoint, SIGNAL(triggered()), this, SLOT(breakAtPoint()));

    m_actionBreakSegment = actionRegistry->makeQAction("path-break-segment", this);
    addAction("path-break-segment", m_actionBreakSegment);
    connect(m_actionBreakSegment, SIGNAL(triggered()), this, SLOT(breakAtSegment()));

    m_actionJoinSegment = actionRegistry->makeQAction("pathpoint-join", this);
    addAction("pathpoint-join", m_actionJoinSegment);
    connect(m_actionJoinSegment, SIGNAL(triggered()), this, SLOT(joinPoints()));

    m_actionMergePoints = actionRegistry->makeQAction("pathpoint-merge", this);
    addAction("pathpoint-merge", m_actionMergePoints);
    connect(m_actionMergePoints, SIGNAL(triggered()), this, SLOT(mergePoints()));

    m_actionConvertToPath = actionRegistry->makeQAction("convert-to-path", this);
    addAction("convert-to-path", m_actionConvertToPath);
    connect(m_actionConvertToPath, SIGNAL(triggered()), this, SLOT(convertToPath()));

    m_contextMenu.reset(new QMenu());


    connect(points, SIGNAL(triggered(QAction*)), this, SLOT(pointTypeChanged(QAction*)));
    connect(&m_pointSelection, SIGNAL(selectionChanged()), this, SLOT(pointSelectionChanged()));

    QBitmap b = QBitmap::fromData(QSize(16, 16), needle_bits);
    QBitmap m = b.createHeuristicMask(false);

    m_selectCursor = QCursor(b, m, 2, 0);

    b = QBitmap::fromData(QSize(16, 16), needle_move_bits);
    m = b.createHeuristicMask(false);

    m_moveCursor = QCursor(b, m, 2, 0);
}

KoPathTool::~KoPathTool()
{
    delete m_activeHandle;
    delete m_activeSegment;
    delete m_currentStrategy;
}

QList<QPointer<QWidget> >  KoPathTool::createOptionWidgets()
{
    QList<QPointer<QWidget> > list;

    PathToolOptionWidget * toolOptions = new PathToolOptionWidget(this);
    connect(this, SIGNAL(typeChanged(int)), toolOptions, SLOT(setSelectionType(int)));
    connect(this, SIGNAL(singleShapeChanged(KoPathShape*)), toolOptions, SLOT(setCurrentShape(KoPathShape*)));
    connect(toolOptions, SIGNAL(sigRequestUpdateActions()), this, SLOT(updateActions()));
    updateOptionsWidget();
    toolOptions->setWindowTitle(i18n("Edit Shape"));
    list.append(toolOptions);

    return list;
}

void KoPathTool::pointTypeChanged(QAction *type)
{
    Q_D(KoToolBase);
    if (m_pointSelection.hasSelection()) {
        QList<KoPathPointData> selectedPoints = m_pointSelection.selectedPointsData();

        KUndo2Command *initialConversionCommand = createPointToCurveCommand(selectedPoints);

        // conversion should happen before the c-tor
        // of KoPathPointTypeCommand is executed!
        if (initialConversionCommand) {
            initialConversionCommand->redo();
        }

        KUndo2Command *command =
            new KoPathPointTypeCommand(selectedPoints,
                                       static_cast<KoPathPointTypeCommand::PointType>(type->data().toInt()));

        if (initialConversionCommand) {
            using namespace KisCommandUtils;
            CompositeCommand *parent = new CompositeCommand();
            parent->setText(command->text());
            parent->addCommand(new SkipFirstRedoWrapper(initialConversionCommand));
            parent->addCommand(command);
            command = parent;
        }

        d->canvas->addCommand(command);
    }
}

void KoPathTool::insertPoints()
{
    Q_D(KoToolBase);
    QList<KoPathPointData> segments(m_pointSelection.selectedSegmentsData());
    if (segments.size() == 1) {
        qreal positionInSegment = 0.5;
        if (m_activeSegment && m_activeSegment->isValid()) {
            positionInSegment = m_activeSegment->positionOnSegment;
        }

        KoPathPointInsertCommand *cmd = new KoPathPointInsertCommand(segments, positionInSegment);
        d->canvas->addCommand(cmd);

        // TODO: this construction is dangerous. The canvas can remove the command right after
        //       it has been added to it!

        KoCanvasUpdatesCollector pendingUpdates(canvas());
        m_pointSelection.clear(pendingUpdates);
        foreach (KoPathPoint * p, cmd->insertedPoints()) {
            m_pointSelection.add(p, false, pendingUpdates);
        }
    }
}

void KoPathTool::removePoints()
{
    Q_D(KoToolBase);
    if (m_pointSelection.size() > 0) {
        KoCanvasUpdatesCollector pendingUpdates(canvas());
        pendingUpdates.addUpdate(activeHandleUpdateRects());

        KUndo2Command *cmd = KoPathPointRemoveCommand::createCommand(m_pointSelection.selectedPointsData(), d->canvas->shapeController());
        PointHandle *pointHandle = dynamic_cast<PointHandle*>(m_activeHandle);
        if (pointHandle && m_pointSelection.contains(pointHandle->activePoint())) {
            delete m_activeHandle;
            m_activeHandle = 0;
        }
        clearActivePointSelectionReferences(pendingUpdates);
        d->canvas->addCommand(cmd);
    }
}

void KoPathTool::pointToLine()
{
    Q_D(KoToolBase);
    if (m_pointSelection.hasSelection()) {
        QList<KoPathPointData> selectedPoints = m_pointSelection.selectedPointsData();
        QList<KoPathPointData> pointToChange;

        QList<KoPathPointData>::const_iterator it(selectedPoints.constBegin());
        for (; it != selectedPoints.constEnd(); ++it) {
            KoPathPoint *point = it->pathShape->pointByIndex(it->pointIndex);
            if (point && (point->activeControlPoint1() || point->activeControlPoint2()))
                pointToChange.append(*it);
        }

        if (! pointToChange.isEmpty()) {
            d->canvas->addCommand(new KoPathPointTypeCommand(pointToChange, KoPathPointTypeCommand::Line));
        }
    }
}

void KoPathTool::pointToCurve()
{
    Q_D(KoToolBase);
    if (m_pointSelection.hasSelection()) {
        QList<KoPathPointData> selectedPoints = m_pointSelection.selectedPointsData();

        KUndo2Command *command = createPointToCurveCommand(selectedPoints);

        if (command) {
            d->canvas->addCommand(command);
        }
    }
}

KUndo2Command* KoPathTool::createPointToCurveCommand(const QList<KoPathPointData> &points)
{
    KUndo2Command *command = 0;
    QList<KoPathPointData> pointToChange;

    QList<KoPathPointData>::const_iterator it(points.constBegin());
    for (; it != points.constEnd(); ++it) {
        KoPathPoint *point = it->pathShape->pointByIndex(it->pointIndex);
        if (point && (! point->activeControlPoint1() || ! point->activeControlPoint2()))
            pointToChange.append(*it);
    }

    if (!pointToChange.isEmpty()) {
        command = new KoPathPointTypeCommand(pointToChange, KoPathPointTypeCommand::Curve);
    }

    return command;
}

void KoPathTool::segmentToLine()
{
    Q_D(KoToolBase);
    if (m_pointSelection.size() > 1) {
        QList<KoPathPointData> segments(m_pointSelection.selectedSegmentsData());
        if (segments.size() > 0) {
            d->canvas->addCommand(new KoPathSegmentTypeCommand(segments, KoPathSegmentTypeCommand::Line));
        }
    }
}

void KoPathTool::segmentToCurve()
{
    Q_D(KoToolBase);
    if (m_pointSelection.size() > 1) {
        QList<KoPathPointData> segments(m_pointSelection.selectedSegmentsData());
        if (segments.size() > 0) {
            d->canvas->addCommand(new KoPathSegmentTypeCommand(segments, KoPathSegmentTypeCommand::Curve));
        }
    }
}

void KoPathTool::convertToPath()
{
    Q_D(KoToolBase);
    QList<KoParameterShape*> shapesToConvert;
    Q_FOREACH (KoShape *shape, m_pointSelection.selectedShapes()) {
        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape())
            shapesToConvert.append(parameterShape);
    }
    if (shapesToConvert.count())
        d->canvas->addCommand(new KoParameterToPathCommand(shapesToConvert));
    updateOptionsWidget();
}

namespace {
bool checkCanJoinToPoints(const KoPathPointData & pd1, const KoPathPointData & pd2)
{
    const KoPathPointIndex & index1 = pd1.pointIndex;
    const KoPathPointIndex & index2 = pd2.pointIndex;

    KoPathShape *path1 = pd1.pathShape;
    KoPathShape *path2 = pd2.pathShape;

    // check if subpaths are already closed
    if (path1->isClosedSubpath(index1.first) || path2->isClosedSubpath(index2.first))
        return false;

    // check if first point is an endpoint
    if (index1.second != 0 && index1.second != path1->subpathPointCount(index1.first)-1)
        return false;

    // check if second point is an endpoint
    if (index2.second != 0 && index2.second != path2->subpathPointCount(index2.first)-1)
        return false;

    return true;
}
}

void KoPathTool::mergePointsImpl(bool doJoin)
{
    Q_D(KoToolBase);

    if (m_pointSelection.size() != 2)
        return;

    QList<KoPathPointData> pointData = m_pointSelection.selectedPointsData();
    if (pointData.size() != 2) return;

    const KoPathPointData & pd1 = pointData.at(0);
    const KoPathPointData & pd2 = pointData.at(1);

    if (!checkCanJoinToPoints(pd1, pd2)) {
        return;
    }

    KoCanvasUpdatesCollector pendingUpdates(canvas());
    clearActivePointSelectionReferences(pendingUpdates);
    pendingUpdates.forceUpdate();

    KUndo2Command *cmd = 0;

    if (doJoin) {
        cmd = new KoMultiPathPointJoinCommand(pd1, pd2, d->canvas->shapeController()->documentBase(), d->canvas->shapeManager()->selection());
    } else {
        cmd = new KoMultiPathPointMergeCommand(pd1, pd2, d->canvas->shapeController()->documentBase(), d->canvas->shapeManager()->selection());
    }
    d->canvas->addCommand(cmd);
}

QVector<QRectF> KoPathTool::pointSelectionUpdateRects()
{
    m_pointSelection.update();

    return KoShapeHandlesCollection::updateDocRects(
        m_pointSelection.collectSelectedHandles(KisHandleStyle::selectedPrimaryHandles()),
        handleRadius());
}

QVector<QRectF> KoPathTool::activeHandleUpdateRects()
{
    if (!m_activeHandle) return QVector<QRectF>();

    KoShapeHandlesCollection handles;
    m_activeHandle->collectHandles(handles, KisHandleStyle::highlightedPrimaryHandles());

    return handles.updateDocRects(handleRadius());
}

QRectF KoPathTool::activeSegmentUpdateRects()
{
    if (!m_activeSegment || !m_activeSegment->isValid()) return QRectF();

    KoPathPointIndex index = m_activeSegment->path->pathPointIndex(m_activeSegment->segmentStart);
    KoPathSegment segment = m_activeSegment->path->segmentByIndex(index).toCubic();

    return m_activeSegment->path->absoluteTransformation(0).mapRect(segment.boundingRect());
}

void KoPathTool::setActiveSegment(KoPathTool::PathSegment *segment, KoCanvasUpdatesCollector &pendingUpdates)
{
    if (segment == m_activeSegment) return;

    pendingUpdates.addUpdate(activeSegmentUpdateRects());
    delete m_activeSegment;
    m_activeSegment = segment;
    pendingUpdates.addUpdate(activeSegmentUpdateRects());
}

void KoPathTool::setActiveHandle(KoPathToolHandle *handle, KoCanvasUpdatesCollector &pendingUpdates)
{
    if (handle == m_activeHandle) return;

    pendingUpdates.addUpdate(activeHandleUpdateRects());
    delete m_activeHandle;
    m_activeHandle = handle;
    pendingUpdates.addUpdate(activeHandleUpdateRects());
}

void KoPathTool::joinPoints()
{
    mergePointsImpl(true);
}

void KoPathTool::mergePoints()
{
    mergePointsImpl(false);
}

void KoPathTool::breakAtPoint()
{
    Q_D(KoToolBase);
    if (m_pointSelection.hasSelection()) {
        d->canvas->addCommand(new KoPathBreakAtPointCommand(m_pointSelection.selectedPointsData()));
    }
}

void KoPathTool::breakAtSegment()
{
    Q_D(KoToolBase);
    // only try to break a segment when 2 points of the same object are selected
    if (m_pointSelection.objectCount() == 1 && m_pointSelection.size() == 2) {
        QList<KoPathPointData> segments(m_pointSelection.selectedSegmentsData());
        if (segments.size() == 1) {
            d->canvas->addCommand(new KoPathSegmentBreakCommand(segments.at(0)));
        }
    }
}

void KoPathTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoToolBase);

    KoShapeHandlesCollection collection;

    collection.addHandles(m_pointSelection.collectShapeHandles());
    collection.addHandles(m_pointSelection.collectSelectedHandles(KisHandleStyle::selectedPrimaryHandles()));

    if (m_activeHandle) {
        if (m_activeHandle->check(m_pointSelection.selectedShapes())) {
            m_activeHandle->collectHandles(collection, KisHandleStyle::highlightedPrimaryHandles());
        }

        KIS_SAFE_ASSERT_RECOVER (m_activeHandle->check(m_pointSelection.selectedShapes())) {
            delete m_activeHandle;
            m_activeHandle = 0;
        }

    } else if (m_activeSegment && m_activeSegment->isValid()) {
        addSegmentHandle(collection, m_activeSegment);
    }

    collection.drawHandles(&painter, converter, m_handleRadius);

    if (m_currentStrategy) {
        KisPaintingTweaks::StateSaver saver(&painter);
        m_currentStrategy->paint(painter, converter);
    }

    if (m_currentStrategy) {
        KisPaintingTweaks::StateSaver saver(&painter);
        KoShape::applyConversion(painter, converter);
        d->canvas->snapGuide()->paint(painter, converter);
    }
}

void KoPathTool::repaintDecorations()
{
//    Q_FOREACH (KoShape *shape, m_pointSelection.selectedShapes()) {
//        repaint(shape->boundingRect());
//    }

//    m_pointSelection.repaint();
//    updateOptionsWidget();
}

void KoPathTool::mousePressEvent(KoPointerEvent *event)
{
    KoCanvasUpdatesCollector pendingUpdates(canvas());

    // we are moving if we hit a point and use the left mouse button
    event->ignore();
    if (m_activeHandle) {
        m_currentStrategy = m_activeHandle->handleMousePress(event);
        event->accept();
    } else {
        if (event->button() & Qt::LeftButton) {

            // check if we hit a path segment
            if (m_activeSegment && m_activeSegment->isValid()) {

                KoPathShape *shape = m_activeSegment->path;
                KoPathPointIndex index = shape->pathPointIndex(m_activeSegment->segmentStart);
                KoPathSegment segment = shape->segmentByIndex(index);

                m_pointSelection.add(segment.first(), !(event->modifiers() & Qt::ShiftModifier), pendingUpdates);
                m_pointSelection.add(segment.second(), false, pendingUpdates);

                KoPathPointData data(shape, index);
                m_currentStrategy = new KoPathSegmentChangeStrategy(this, event->point, data, m_activeSegment->positionOnSegment);
                event->accept();
            } else {

                KoShapeManager *shapeManager = canvas()->shapeManager();
                KoSelection *selection = shapeManager->selection();

                KoShape *shape = shapeManager->shapeAt(event->point, KoFlake::ShapeOnTop);
                if (shape && !selection->isSelected(shape)) {

                    if (!(event->modifiers() & Qt::ShiftModifier)) {
                        selection->deselectAll();
                    }

                    selection->select(shape);
                } else {
                    KIS_ASSERT_RECOVER_RETURN(m_currentStrategy == 0);
                    m_currentStrategy = new KoPathPointRubberSelectStrategy(this, event->point);
                    event->accept();
                }
            }
        }
    }
}

void KoPathTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (event->button() & Qt::RightButton)
        return;

    KoCanvasUpdatesCollector pendingUpdates(canvas());

    if (m_currentStrategy) {
        pendingUpdates.addUpdate(pointSelectionUpdateRects());
        pendingUpdates.addUpdate(activeHandleUpdateRects());
        pendingUpdates.addUpdate(activeSegmentUpdateRects());

        m_lastPoint = event->point;
        m_currentStrategy->handleMouseMove(event->point, event->modifiers());

        pendingUpdates.addUpdate(pointSelectionUpdateRects());
        pendingUpdates.addUpdate(activeHandleUpdateRects());
        pendingUpdates.addUpdate(activeSegmentUpdateRects());

        return;
    }

    {
        QScopedPointer<KoPathToolHandle> newActiveHandle;
        bool isParameterShape = false;

        const QRectF roi = handleGrabDocRect(event->point);
        const qreal docDistanceThreshold = 0.5 * KisAlgebra2D::maxDimension(roi);
        qreal minDistance = std::numeric_limits<qreal>::max();


        Q_FOREACH (KoPathShape *shape, m_pointSelection.selectedShapes()) {
            KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>(shape);
            if (parameterShape && parameterShape->isParametricShape()) {

                for (int i = 0; i < parameterShape->handleCount(); i++) {
                    const QPointF docPoint = parameterShape->absoluteTransformation(0).map(parameterShape->handlePosition(i));
                    const qreal distance = kisDistance(docPoint, event->point);

                    if (distance < docDistanceThreshold && distance < minDistance) {
                        isParameterShape = true;
                        minDistance = distance;

                        if (KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>(parameterShape)) {
                            newActiveHandle.reset(new ConnectionHandle(this, connectionShape, i));
                        } else {
                            newActiveHandle.reset(new ParameterHandle(this, parameterShape, i));
                        }
                    }
                }

            } else {
                const QRectF localROI = shape->documentToShape(roi);

                Q_FOREACH (KoPathPoint *p, shape->allPathPoints()) {
                    auto addIfSmaller =
                        [&] (const QPointF &localPoint, KoPathPoint::PointType type, KoPathPoint *parentPoint) {

                            const QPointF docPoint = shape->absoluteTransformation(0).map(localPoint);
                            const qreal distance = kisDistance(docPoint, event->point);

                            if (distance < docDistanceThreshold && distance < minDistance) {
                                isParameterShape = false;
                                minDistance = distance;
                                newActiveHandle.reset(new PointHandle(this, parentPoint, type));
                            }
                        };

                    // check for the control points first as otherwise it is no longer
                    // possible to change the control points when they are the same as the point
                    if (m_pointSelection.contains(p)) {
                        if (p->activeControlPoint1() && localROI.contains(p->controlPoint1())) {
                            addIfSmaller(p->controlPoint1(), KoPathPoint::ControlPoint1, p);
                        }

                        if (p->activeControlPoint2() && localROI.contains(p->controlPoint2())) {
                            addIfSmaller(p->controlPoint2(), KoPathPoint::ControlPoint2, p);
                        }
                    }

                    // check the node point at last
                    if (localROI.contains(p->point())) {
                        addIfSmaller(p->point(), KoPathPoint::Node, p);
                    }
                }
            }
        }

        if (newActiveHandle) {
            useCursor(m_moveCursor);

            if (isParameterShape) {
                emit statusTextChanged(i18n("Drag to move handle."));
            } else {
                PointHandle *handle = static_cast<PointHandle*>(newActiveHandle.data());
                if (handle->activePointType() == KoPathPoint::Node) {
                    emit statusTextChanged(i18n("Drag to move point. Shift click to change point type."));
                } else {
                    emit statusTextChanged(i18n("Drag to move control point."));
                }
            }

            if (!m_activeHandle || !m_activeHandle->compareTo(newActiveHandle.data())) {
                setActiveSegment(0, pendingUpdates);
                setActiveHandle(newActiveHandle.take(), pendingUpdates);
            }

            return;
        }
    }

    useCursor(m_selectCursor);

    setActiveHandle(0, pendingUpdates);

    PathSegment *hoveredSegment = segmentAtPoint(event->point, m_activeSegment);
    if (hoveredSegment) {
        useCursor(Qt::PointingHandCursor);
        emit statusTextChanged(i18n("Drag to change curve directly. Double click to insert new path point."));
        setActiveSegment(hoveredSegment, pendingUpdates);
    } else {
        setActiveSegment(0, pendingUpdates);

        uint selectedPointCount = m_pointSelection.size();
        if (selectedPointCount == 0)
            emit statusTextChanged(QString());
        else if (selectedPointCount == 1)
            emit statusTextChanged(i18n("Press B to break path at selected point."));
        else
            emit statusTextChanged(i18n("Press B to break path at selected segments."));
    }
}

void KoPathTool::addSegmentHandle(KoShapeHandlesCollection &handles, PathSegment *pathSegment)
{
    if (!pathSegment || !pathSegment->isValid()) return;

    KoPathPointIndex index = pathSegment->path->pathPointIndex(pathSegment->segmentStart);
    KoPathSegment segment = pathSegment->path->segmentByIndex(index).toCubic();

    QPainterPath path;
    path.moveTo(segment.first()->point());
    path.cubicTo(segment.first()->controlPoint2(),
                 segment.second()->controlPoint1(),
                 segment.second()->point());


    handles.addHandles(pathSegment->path, KisHandleStyle::secondarySelection(),
                       KritaUtils::Handle(KritaUtils::OutlinePath, path));
}


void KoPathTool::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_D(KoToolBase);
    if (m_currentStrategy) {
        const bool hadNoSelection = !m_pointSelection.hasSelection();
        m_currentStrategy->finishInteraction(event->modifiers());
        KUndo2Command *command = m_currentStrategy->createCommand();
        if (command)
            d->canvas->addCommand(command);
        if (hadNoSelection && dynamic_cast<KoPathPointRubberSelectStrategy*>(m_currentStrategy)
                && !m_pointSelection.hasSelection()) {
            // the click didn't do anything at all. Allow it to be used by others.
            event->ignore();
        }
        delete m_currentStrategy;
        m_currentStrategy = 0;
    }
}

void KoPathTool::keyPressEvent(QKeyEvent *event)
{
    Q_D(KoToolBase);
    if (m_currentStrategy) {
        switch (event->key()) {
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Shift:
        case Qt::Key_Meta:
            if (! event->isAutoRepeat()) {
                m_currentStrategy->handleMouseMove(m_lastPoint, event->modifiers());
            }
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
    } else {
        switch (event->key()) {
#ifndef NDEBUG
        case Qt::Key_D:
            if (m_pointSelection.objectCount() == 1) {
                QList<KoPathPointData> selectedPoints = m_pointSelection.selectedPointsData();
                KoPathShapePrivate *p = static_cast<KoPathShapePrivate*>(selectedPoints[0].pathShape->priv());
                p->debugPath();
            }
            break;
#endif
        case Qt::Key_B:
            if (m_pointSelection.size() == 1)
                breakAtPoint();
            else if (m_pointSelection.size() >= 2)
                breakAtSegment();
            break;
        default:
            event->ignore();
            return;
        }
    }
    event->accept();
}

void KoPathTool::keyReleaseEvent(QKeyEvent *event)
{
    if (m_currentStrategy) {
        switch (event->key()) {
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Shift:
        case Qt::Key_Meta:
            if (! event->isAutoRepeat()) {
                m_currentStrategy->handleMouseMove(m_lastPoint, Qt::NoModifier);
            }
            break;
        default:
            break;
        }
    }
    event->accept();
}

void KoPathTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    Q_D(KoToolBase);
    event->ignore();

    // check if we are doing something else at the moment
    if (m_currentStrategy) return;

    QScopedPointer<PathSegment> s(segmentAtPoint(event->point, 0));

    if (s && s->isValid()) {
        QList<KoPathPointData> segments;
        segments.append(KoPathPointData(s->path, s->path->pathPointIndex(s->segmentStart)));
        KoPathPointInsertCommand *cmd = new KoPathPointInsertCommand(segments, s->positionOnSegment);
        d->canvas->addCommand(cmd);

        // TODO: this construction is dangerous. The canvas can remove the command right after
        //       it has been added to it!

        KoCanvasUpdatesCollector pendingUpdates(canvas());
        foreach (KoPathPoint * p, cmd->insertedPoints()) {
            m_pointSelection.add(p, false, pendingUpdates);
        }
        updateActions();
        event->accept();
    } else if (!m_activeHandle && !m_activeSegment && m_activatedTemporarily) {
        emit done();
    }
}

KoPathTool::PathSegment* KoPathTool::segmentAtPoint(const QPointF &point, PathSegment *excludeSegment)
{
    Q_D(KoToolBase);

    // TODO: use drag distance!
    const int clickProximity = 5;

    // convert click proximity to point using the current zoom level
    QPointF clickOffset = d->canvas->viewConverter()->viewToDocument(QPointF(clickProximity, clickProximity));
    // the max allowed distance from a segment
    const qreal maxSquaredDistance = clickOffset.x()*clickOffset.x();

    QScopedPointer<PathSegment> segment(new PathSegment);
    PathSegment *resultSegment = 0;

    Q_FOREACH (KoPathShape *shape, m_pointSelection.selectedShapes()) {
        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape())
            continue;

        // convert document point to shape coordinates
        QPointF p = shape->documentToShape(point);
        // our region of interest, i.e. a region around our mouse position
        QRectF roi(p - clickOffset, p + clickOffset);

        qreal minSqaredDistance = HUGE_VAL;
        // check all segments of this shape which intersect the region of interest
        QList<KoPathSegment> segments = shape->segmentsAt(roi);
        foreach (const KoPathSegment &s, segments) {
            qreal nearestPointParam = s.nearestPoint(p);
            QPointF nearestPoint = s.pointAt(nearestPointParam);
            QPointF diff = p - nearestPoint;
            qreal squaredDistance = diff.x()*diff.x() + diff.y()*diff.y();
            // are we within the allowed distance ?
            if (squaredDistance > maxSquaredDistance)
                continue;
            // are we closer to the last closest point ?
            if (squaredDistance < minSqaredDistance) {
                if (excludeSegment &&
                    excludeSegment->path == shape &&
                    excludeSegment->segmentStart == s.first()) {

                    excludeSegment->positionOnSegment = nearestPointParam;
                    resultSegment = excludeSegment;
                } else {
                    segment->path = shape;
                    segment->segmentStart = s.first();
                    segment->positionOnSegment = nearestPointParam;
                    resultSegment = segment.data();
                }
            }
        }
    }

    return resultSegment == segment.data() ? segment.take() : resultSegment;
}

void KoPathTool::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KoToolBase::activate(activation, shapes);

    Q_D(KoToolBase);

    m_activatedTemporarily = activation == TemporaryActivation;

    // retrieve the actual global handle radius
    m_handleRadius = handleRadius();
    d->canvas->snapGuide()->reset();

    useCursor(m_selectCursor);
    m_canvasConnections.addConnection(d->canvas->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
    m_canvasConnections.addConnection(d->canvas->selectedShapesProxy(), SIGNAL(selectionContentChanged()), this, SLOT(updateActions()));

    initializeWithShapes(shapes.toList());
}

void KoPathTool::slotSelectionChanged()
{
    Q_D(KoToolBase);
    QList<KoShape*> shapes =
        d->canvas->selectedShapesProxy()->selection()->selectedEditableShapesAndDelegates();

    initializeWithShapes(shapes);
}

void KoPathTool::clearActivePointSelectionReferences(KoCanvasUpdatesCollector &pendingUpdates)
{
    setActiveHandle(0, pendingUpdates);
    setActiveSegment(0, pendingUpdates);

    m_pointSelection.clear(pendingUpdates);
}

void KoPathTool::initializeWithShapes(const QList<KoShape*> shapes)
{
    QList<KoPathShape*> selectedShapes;
    Q_FOREACH (KoShape *shape, shapes) {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape);

        if (pathShape && pathShape->isShapeEditable()) {
            selectedShapes.append(pathShape);
        }
    }

    KoCanvasUpdatesCollector pendingUpdates(canvas());

    if (selectedShapes != m_pointSelection.selectedShapes()) {
        pendingUpdates.addUpdate(
            KoShapeHandlesCollection::updateDocRects(
                m_pointSelection.collectShapeHandles(), handleRadius()));

        clearActivePointSelectionReferences(pendingUpdates);
        m_pointSelection.setSelectedShapes(selectedShapes);

        pendingUpdates.addUpdate(
            KoShapeHandlesCollection::updateDocRects(
                m_pointSelection.collectShapeHandles(), handleRadius()));

        pendingUpdates.addUpdate(pointSelectionUpdateRects());
    }

    Q_FOREACH (KoPathShape *shape, selectedShapes) {
        // as the tool is just in activation repaintDecorations does not yet get called
        // so we need to use repaint of the tool and it is only needed to repaint the
        // current canvas
        pendingUpdates.addUpdate(shape->boundingRect());
    }

    updateOptionsWidget();
    updateActions();
}

void KoPathTool::updateOptionsWidget()
{
    PathToolOptionWidget::Types type;
    QList<KoPathShape*> selectedShapes = m_pointSelection.selectedShapes();
    Q_FOREACH (KoPathShape *shape, selectedShapes) {
        KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>(shape);
        type |= parameterShape && parameterShape->isParametricShape() ?
                PathToolOptionWidget::ParametricShape : PathToolOptionWidget::PlainPath;
    }

    emit singleShapeChanged(selectedShapes.size() == 1 ? selectedShapes.first() : 0);
    emit typeChanged(type);
}

void KoPathTool::updateActions()
{
    QList<KoPathPointData> pointData = m_pointSelection.selectedPointsData();

    bool canBreakAtPoint = false;

    bool hasNonSmoothPoints = false;
    bool hasNonSymmetricPoints = false;
    bool hasNonSplitPoints = false;

    bool hasNonLinePoints = false;
    bool hasNonCurvePoints = false;

    bool canJoinSubpaths = false;

    if (!pointData.isEmpty()) {
        Q_FOREACH (const KoPathPointData &pd, pointData) {
            const int subpathIndex = pd.pointIndex.first;
            const int pointIndex = pd.pointIndex.second;

            canBreakAtPoint |= pd.pathShape->isClosedSubpath(subpathIndex) ||
                (pointIndex > 0 && pointIndex < pd.pathShape->subpathPointCount(subpathIndex) - 1);

            KoPathPoint *point = pd.pathShape->pointByIndex(pd.pointIndex);

            hasNonSmoothPoints |= !(point->properties() & KoPathPoint::IsSmooth);
            hasNonSymmetricPoints |= !(point->properties() & KoPathPoint::IsSymmetric);
            hasNonSplitPoints |=
                point->properties() & KoPathPoint::IsSymmetric ||
                point->properties() & KoPathPoint::IsSmooth;

            hasNonLinePoints |= point->activeControlPoint1() || point->activeControlPoint2();
            hasNonCurvePoints |= !point->activeControlPoint1() && !point->activeControlPoint2();
        }

        if (pointData.size() == 2) {
            const KoPathPointData & pd1 = pointData.at(0);
            const KoPathPointData & pd2 = pointData.at(1);

            canJoinSubpaths = checkCanJoinToPoints(pd1, pd2);
        }
    }

    m_actionPathPointCorner->setEnabled(hasNonSplitPoints);
    m_actionPathPointSmooth->setEnabled(hasNonSmoothPoints);
    m_actionPathPointSymmetric->setEnabled(hasNonSymmetricPoints);

    m_actionRemovePoint->setEnabled(!pointData.isEmpty());

    m_actionBreakPoint->setEnabled(canBreakAtPoint);

    m_actionCurvePoint->setEnabled(hasNonCurvePoints);
    m_actionLinePoint->setEnabled(hasNonLinePoints);

    m_actionJoinSegment->setEnabled(canJoinSubpaths);
    m_actionMergePoints->setEnabled(canJoinSubpaths);

    QList<KoPathPointData> segments(m_pointSelection.selectedSegmentsData());


    bool canSplitAtSegment = false;
    bool canConvertSegmentToLine = false;
    bool canConvertSegmentToCurve= false;

    if (!segments.isEmpty()) {

        canSplitAtSegment = segments.size() == 1;

        bool hasLines = false;
        bool hasCurves = false;

        Q_FOREACH (const KoPathPointData &pd, segments) {
            KoPathSegment segment = pd.pathShape->segmentByIndex(pd.pointIndex);
            hasLines |= segment.degree() == 1;
            hasCurves |= segment.degree() > 1;
        }

        canConvertSegmentToLine = !segments.isEmpty() && hasCurves;
        canConvertSegmentToCurve= !segments.isEmpty() && hasLines;
    }

    m_actionAddPoint->setEnabled(canSplitAtSegment);

    m_actionLineSegment->setEnabled(canConvertSegmentToLine);
    m_actionCurveSegment->setEnabled(canConvertSegmentToCurve);

    m_actionBreakSegment->setEnabled(canSplitAtSegment);

}

void KoPathTool::deactivate()
{
    Q_D(KoToolBase);

    KoCanvasUpdatesCollector pendingUpdates(canvas());
    clearActivePointSelectionReferences(pendingUpdates);

    m_canvasConnections.clear();
    delete m_currentStrategy;
    m_currentStrategy = 0;
    d->canvas->snapGuide()->reset();

    KoToolBase::deactivate();
}

void KoPathTool::documentResourceChanged(int key, const QVariant & res)
{
    if (key == KoDocumentResourceManager::HandleRadius) {
        int oldHandleRadius = m_handleRadius;

        m_handleRadius = res.toUInt();

        KoCanvasUpdatesCollector pendingUpdates(canvas());

        // repaint with the bigger of old and new handle radius
        int maxRadius = qMax(m_handleRadius, oldHandleRadius);
        Q_FOREACH (KoPathShape *shape, m_pointSelection.selectedShapes()) {
            const QRectF controlPointRect = shape->absoluteTransformation(0).map(shape->outline()).controlPointRect();
            pendingUpdates.addUpdate(kisGrowRect(controlPointRect, maxRadius));
        }
    }
}

void KoPathTool::pointSelectionChanged()
{
    Q_D(KoToolBase);
    updateActions();
    d->canvas->snapGuide()->setIgnoredPathPoints(m_pointSelection.selectedPoints().toList());
    emit selectionChanged(m_pointSelection.hasSelection());
}

namespace {
void addActionsGroupIfEnabled(QMenu *menu, QAction *a1, QAction *a2)
{
    if (a1->isEnabled() || a2->isEnabled()) {
        menu->addAction(a1);
        menu->addAction(a2);
        menu->addSeparator();
    }
}

void addActionsGroupIfEnabled(QMenu *menu, QAction *a1, QAction *a2, QAction *a3)
{
    if (a1->isEnabled() || a2->isEnabled()) {
        menu->addAction(a1);
        menu->addAction(a2);
        menu->addAction(a3);
        menu->addSeparator();
    }
}
}

QMenu *KoPathTool::popupActionsMenu()
{
    if (m_activeHandle) {
        m_activeHandle->trySelectHandle();
    }

    if (m_activeSegment && m_activeSegment->isValid()) {
        KoPathShape *shape = m_activeSegment->path;
        KoPathSegment segment = shape->segmentByIndex(shape->pathPointIndex(m_activeSegment->segmentStart));

        // TODO: it should be done in mouse press part!
        KoCanvasUpdatesCollector pendingUpdates(canvas());

        m_pointSelection.add(segment.first(), true, pendingUpdates);
        m_pointSelection.add(segment.second(), false, pendingUpdates);
    }

    if (m_contextMenu) {
        m_contextMenu->clear();

        addActionsGroupIfEnabled(m_contextMenu.data(),
                                 m_actionPathPointCorner,
                                 m_actionPathPointSmooth,
                                 m_actionPathPointSymmetric);

        addActionsGroupIfEnabled(m_contextMenu.data(),
                                 m_actionCurvePoint,
                                 m_actionLinePoint);

        addActionsGroupIfEnabled(m_contextMenu.data(),
                                 m_actionAddPoint,
                                 m_actionRemovePoint);

        addActionsGroupIfEnabled(m_contextMenu.data(),
                                 m_actionLineSegment,
                                 m_actionCurveSegment);

        addActionsGroupIfEnabled(m_contextMenu.data(),
                                 m_actionBreakPoint,
                                 m_actionBreakSegment);

        addActionsGroupIfEnabled(m_contextMenu.data(),
                                 m_actionJoinSegment,
                                 m_actionMergePoints);

        m_contextMenu->addAction(m_actionConvertToPath);

        m_contextMenu->addSeparator();
    }

    return m_contextMenu.data();
}

void KoPathTool::deleteSelection()
{
    removePoints();
}

KoToolSelection * KoPathTool::selection()
{
    return &m_pointSelection;
}

void KoPathTool::requestUndoDuringStroke()
{
    // noop!
}

void KoPathTool::requestStrokeCancellation()
{
    explicitUserStrokeEndRequest();
}

void KoPathTool::requestStrokeEnd()
{
    // noop!
}

void KoPathTool::explicitUserStrokeEndRequest()
{
    if (m_activatedTemporarily) {
        emit done();
    }
}
