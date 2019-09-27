/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "ConnectionTool.h"

#include <QPointF>
#include <QKeyEvent>
#include <QPainter>

#include "AddConnectionPointCommand.h"
#include "RemoveConnectionPointCommand.h"
#include "ChangeConnectionPointCommand.h"
#include "MoveConnectionPointStrategy.h"
#include "ConnectionPointWidget.h"

#define TextShape_SHAPEID "TextShapeID"

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoShapeFactoryBase.h>
#include <KoShape.h>
#include <KoShapeGroup.h>
#include <KoShapeController.h>
#include <KoShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoSelection.h>
#include <KoPathSegment.h>
#include <KoDocumentResourceManager.h>
#include <KoInteractionStrategy.h>
#include <KoShapeConfigWidgetBase.h>
#include <KoConnectionShapeConfigWidget.h>
#include <KoPathConnectionPointStrategy.h>
#include <KoStrokeConfigWidget.h>
#include <KisHandlePainterHelper.h>

#include "kis_document_aware_spin_box_unit_manager.h"

#include <KoIcon.h>
#include "kis_action_registry.h"

#include <QAction>
#include <klocalizedstring.h>
#include <QDebug>
#include <KoResourcePaths.h>
#include <kundo2command.h>

#include <math.h>

ConnectionTool::ConnectionTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_editMode(Idle)
    , m_connectionType(KoConnectionShape::Standard)
    , m_currentShape(0)
    , m_activeHandle(-1)
    , m_currentStrategy(0)
    , m_oldSnapStrategies(0)
    , m_resetPaint(true)
{
    QPixmap connectPixmap;
    connectPixmap.load(":/cursor_connect.png");
    m_connectCursor = QCursor(connectPixmap, 4, 1);

    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    m_editConnectionPoint = actionRegistry->makeQAction("toggle-edit-mode", this);
    m_editConnectionPoint->setCheckable(true);
//    addAction("toggle-edit-mode", m_editConnectionPoint);

    m_alignPercent = actionRegistry->makeQAction("align-relative", this);
    m_alignPercent->setCheckable(true);
//    addAction("align-relative", m_alignPercent);
    m_alignLeft = actionRegistry->makeQAction("align-left", this);
    m_alignLeft->setCheckable(true);
//    addAction("align-left", m_alignLeft);
    m_alignCenterH = actionRegistry->makeQAction("align-centerh", this);
    m_alignCenterH->setCheckable(true);
//    addAction("align-centerh", m_alignCenterH);
    m_alignRight = actionRegistry->makeQAction("align-right", this);
    m_alignRight->setCheckable(true);
//    addAction("align-right", m_alignRight);
    m_alignTop = actionRegistry->makeQAction("align-top", this);
    m_alignTop->setCheckable(true);
//    addAction("align-top", m_alignTop);
    m_alignCenterV = actionRegistry->makeQAction("align-centerv", this);
    m_alignCenterV->setCheckable(true);
//    addAction("align-centerv", m_alignCenterV);
    m_alignBottom = actionRegistry->makeQAction("align-bottom", this);
    m_alignBottom->setCheckable(true);
//    addAction("align-bottom", m_alignBottom);

    m_escapeAll = actionRegistry->makeQAction("escape-all", this);
    m_escapeAll->setCheckable(true);
//    addAction("escape-all", m_escapeAll);
    m_escapeHorizontal = actionRegistry->makeQAction("escape-horizontal", this);
    m_escapeHorizontal->setCheckable(true);
//    addAction("escape-horizontal", m_escapeHorizontal);
    m_escapeVertical = actionRegistry->makeQAction("escape-vertical", this);
    m_escapeVertical->setCheckable(true);
//    addAction("escape-vertical", m_escapeVertical);
    m_escapeLeft = actionRegistry->makeQAction("escape-left", this);
    m_escapeLeft->setCheckable(true);
//    addAction("escape-left", m_escapeLeft);
    m_escapeRight = actionRegistry->makeQAction("escape-right", this);
    m_escapeRight->setCheckable(true);
//    addAction("escape-right", m_escapeRight);
    m_escapeUp = actionRegistry->makeQAction("escape-up", this);
    m_escapeUp->setCheckable(true);
//    addAction("escape-up", m_escapeUp);
    m_escapeDown = actionRegistry->makeQAction("escape-down", this);
    m_escapeDown->setCheckable(true);
//    addAction("escape-down", m_escapeDown);

    m_alignHorizontal = new QActionGroup(this);
    m_alignHorizontal->setExclusive(true);
    m_alignHorizontal->addAction(m_alignLeft);
    m_alignHorizontal->addAction(m_alignCenterH);
    m_alignHorizontal->addAction(m_alignRight);
    connect(m_alignHorizontal, SIGNAL(triggered(QAction*)), this, SLOT(horizontalAlignChanged()));

    m_alignVertical = new QActionGroup(this);
    m_alignVertical->setExclusive(true);
    m_alignVertical->addAction(m_alignTop);
    m_alignVertical->addAction(m_alignCenterV);
    m_alignVertical->addAction(m_alignBottom);
    connect(m_alignVertical, SIGNAL(triggered(QAction*)), this, SLOT(verticalAlignChanged()));

    m_alignRelative = new QActionGroup(this);
    m_alignRelative->setExclusive(true);
    m_alignRelative->addAction(m_alignPercent);
    connect(m_alignRelative, SIGNAL(triggered(QAction*)), this, SLOT(relativeAlignChanged()));

    m_escapeDirections = new QActionGroup(this);
    m_escapeDirections->setExclusive(true);
    m_escapeDirections->addAction(m_escapeAll);
    m_escapeDirections->addAction(m_escapeHorizontal);
    m_escapeDirections->addAction(m_escapeVertical);
    m_escapeDirections->addAction(m_escapeLeft);
    m_escapeDirections->addAction(m_escapeRight);
    m_escapeDirections->addAction(m_escapeUp);
    m_escapeDirections->addAction(m_escapeDown);
    connect(m_escapeDirections, SIGNAL(triggered(QAction*)), this, SLOT(escapeDirectionChanged()));

    connect(this, SIGNAL(connectionPointEnabled(bool)), m_alignHorizontal, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(connectionPointEnabled(bool)), m_alignVertical, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(connectionPointEnabled(bool)), m_alignRelative, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(connectionPointEnabled(bool)), m_escapeDirections, SLOT(setEnabled(bool)));

    resetEditMode();
}

ConnectionTool::~ConnectionTool()
{
}

void ConnectionTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    // get the correctly sized rect for painting handles
    QRectF handleRect = handlePaintRect(QPointF());

    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_currentStrategy) {
        painter.save();
        m_currentStrategy->paint(painter, converter);
        painter.restore();
    }

    QList<KoShape *> shapes = canvas()->shapeManager()->shapes();
    for (QList<KoShape *>::const_iterator end = shapes.constBegin(); end !=  shapes.constEnd(); ++end) {
        KoShape *shape = *end;
        if (!dynamic_cast<KoConnectionShape *>(shape)) {
            // only paint connection points of textShapes not inside a tos container and other shapes
            if (shape->shapeId() == TextShape_SHAPEID && dynamic_cast<KoTosContainer *>(shape->parent())) {
                continue;
            }

            painter.save();
            painter.setPen(Qt::black);
            QTransform transform = shape->absoluteTransformation(0);
            KoShape::applyConversion(painter, converter);
            // Draw all the connection points of the shape
            KoConnectionPoints connectionPoints = shape->connectionPoints();
            KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
            KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
            for (; cp != lastCp; ++cp) {
                if (shape == findNonConnectionShapeAtPosition(transform.map(cp.value().position))) {
                    handleRect.moveCenter(transform.map(cp.value().position));
                    painter.setBrush(cp.key() == m_activeHandle && shape == m_currentShape ?
                                         Qt::red : Qt::white);
                    painter.drawRect(handleRect);
                }
            }
            painter.restore();
        }
    }
    // paint connection points or connection handles depending
    // on the shape the mouse is currently
    if (m_currentShape && m_editMode == EditConnection) {
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(m_currentShape);
        if (connectionShape) {
            int radius = handleRadius() + 1;
            int handleCount = connectionShape->handleCount();
            for (int i = 0; i < handleCount; ++i) {
                KisHandlePainterHelper helper = KoShape::createHandlePainterHelper(&painter, connectionShape, converter, radius);
                helper.setHandleStyle(i == m_activeHandle ? KisHandleStyle::highlightedPrimaryHandles() : KisHandleStyle::primarySelection());
                connectionShape->paintHandle(helper, i);
            }
        }
    }
}

void ConnectionTool::repaintDecorations()
{
    const qreal radius = handleRadius();
    QRectF repaintRect;

    if (m_currentShape) {
        repaintRect = m_currentShape->boundingRect();
        canvas()->updateCanvas(repaintRect.adjusted(-radius, -radius, radius, radius));
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(m_currentShape);
        if (!m_resetPaint && m_currentShape->isVisible() && !connectionShape) {
            // only paint connection points of textShapes not inside a tos container and other shapes
            if (!(m_currentShape->shapeId() == TextShape_SHAPEID &&
                  dynamic_cast<KoTosContainer *>(m_currentShape->parent()))) {
                KoConnectionPoints connectionPoints = m_currentShape->connectionPoints();
                KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
                KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
                for (; cp != lastCp; ++cp) {
                    repaintRect = handleGrabRect(m_currentShape->shapeToDocument(cp.value().position));
                    canvas()->updateCanvas(repaintRect.adjusted(-radius, -radius, radius, radius));
                }
            }
        }
        if (m_editMode == EditConnection) {
            if (connectionShape) {
                QPointF handlePos = connectionShape->handlePosition(m_activeHandle);
                handlePos = connectionShape->shapeToDocument(handlePos);
                repaintRect = handlePaintRect(handlePos);
                canvas()->updateCanvas(repaintRect.adjusted(-radius, -radius, radius, radius));
            }
        }
    }
    if (m_resetPaint) {
        QList<KoShape *> shapes = canvas()->shapeManager()->shapes();
        for (QList<KoShape *>::const_iterator end = shapes.constBegin(); end !=  shapes.constEnd(); ++end) {
            KoShape *shape = *end;
            if (!dynamic_cast<KoConnectionShape *>(shape)) {
                // only paint connection points of textShapes not inside a tos container and other shapes
                if (shape->shapeId() == TextShape_SHAPEID && dynamic_cast<KoTosContainer *>(shape->parent())) {
                    continue;
                }

                KoConnectionPoints connectionPoints = shape->connectionPoints();
                KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
                KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
                for (; cp != lastCp; ++cp) {
                    repaintRect = handleGrabRect(shape->shapeToDocument(cp.value().position));
                    canvas()->updateCanvas(repaintRect.adjusted(-radius, -radius, radius, radius));
                }
            }
        }
    }
    m_resetPaint = false;
}

void ConnectionTool::mousePressEvent(KoPointerEvent *event)
{

    if (!m_currentShape) {
        return;
    }

    KoShape *hitShape = findShapeAtPosition(event->point);
    int hitHandle = handleAtPoint(m_currentShape, event->point);

    if (m_editMode == EditConnection && hitHandle >= 0) {
        // create connection handle change strategy
        m_currentStrategy = new KoPathConnectionPointStrategy(this, dynamic_cast<KoConnectionShape *>(m_currentShape), hitHandle);
    } else if (m_editMode == EditConnectionPoint) {
        if (hitHandle >= KoConnectionPoint::FirstCustomConnectionPoint) {
            // start moving custom connection point
            m_currentStrategy = new MoveConnectionPointStrategy(m_currentShape, hitHandle, this);
        }
    } else if (m_editMode == CreateConnection) {
        // create new connection shape, connect it to the active connection point
        // and start editing the new connection
        // create the new connection shape
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
        KoShape *shape = factory->createDefaultShape(canvas()->shapeController()->resourceManager());
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(shape);
        if (!connectionShape) {
            delete shape;
            resetEditMode();
            return;
        }
        //set connection type
        connectionShape->setType(m_connectionType);
        // get the position of the connection point we start our connection from
        QPointF cp = m_currentShape->shapeToDocument(m_currentShape->connectionPoint(m_activeHandle).position);
        // move both handles to that point
        connectionShape->moveHandle(0, cp);
        connectionShape->moveHandle(1, cp);
        // connect the first handle of the connection shape to our connection point
        if (!connectionShape->connectFirst(m_currentShape, m_activeHandle)) {
            delete shape;
            resetEditMode();
            return;
        }
        //add connector label
        connectionShape->createTextShape(canvas()->shapeController()->resourceManager());
        connectionShape->setPlainText(QString());
        // create the connection edit strategy from the path tool
        m_currentStrategy = new KoPathConnectionPointStrategy(this, connectionShape, 1);
        if (!m_currentStrategy) {
            delete shape;
            resetEditMode();
            return;
        }
        // update our handle data
        setEditMode(m_editMode, shape, 1);
        // add connection shape to the shape manager so it gets painted
        canvas()->shapeManager()->addShape(connectionShape);
    } else {
        // pressing on a shape in idle mode switches to corresponding edit mode
        if (hitShape) {
            if (dynamic_cast<KoConnectionShape *>(hitShape)) {
                int hitHandle = handleAtPoint(hitShape, event->point);
                setEditMode(EditConnection, hitShape, hitHandle);
                if (hitHandle >= 0) {
                    // start editing connection shape
                    m_currentStrategy = new KoPathConnectionPointStrategy(this, dynamic_cast<KoConnectionShape *>(m_currentShape), m_activeHandle);
                }
            }
        } else {
            resetEditMode();
        }
    }
}

void ConnectionTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        repaintDecorations();
        if (m_editMode != EditConnection && m_editMode != CreateConnection) {
            QPointF snappedPos = canvas()->snapGuide()->snap(event->point, event->modifiers());
            m_currentStrategy->handleMouseMove(snappedPos, event->modifiers());
        } else {
            m_currentStrategy->handleMouseMove(event->point, event->modifiers());
        }
        repaintDecorations();
    } else if (m_editMode == EditConnectionPoint) {
        KoShape *hoverShape = findNonConnectionShapeAtPosition(event->point);//TODO exclude connectors, need snap guide maybe?
        if (hoverShape) {
            m_currentShape = hoverShape;
            Q_ASSERT(m_currentShape);
            // check if we should highlight another connection point
            int handle = handleAtPoint(m_currentShape, event->point);
            if (handle >= 0) {
                setEditMode(m_editMode, m_currentShape, handle);
                useCursor(handle >= KoConnectionPoint::FirstCustomConnectionPoint ? Qt::SizeAllCursor : Qt::ArrowCursor);
            } else {
                updateStatusText();
                useCursor(Qt::CrossCursor);
            }
        } else {
            m_currentShape = 0;
            useCursor(Qt::ArrowCursor);
        }
    } else if (m_editMode == EditConnection) {
        Q_ASSERT(m_currentShape);
        KoShape *hoverShape = findShapeAtPosition(event->point);
        // check if we should highlight another connection handle
        int handle = handleAtPoint(m_currentShape, event->point);
        setEditMode(m_editMode, m_currentShape, handle);
        if (m_activeHandle == KoConnectionShape::StartHandle ||
                m_activeHandle == KoConnectionShape::EndHandle) {
            useCursor(Qt::SizeAllCursor);
        } else if (m_activeHandle >= KoConnectionShape::ControlHandle_1) {

        } else if (hoverShape && hoverShape != m_currentShape) {
            useCursor(Qt::PointingHandCursor);
        } else {
            useCursor(Qt::ArrowCursor);
        }
    } else {// Idle and no current strategy
        KoShape *hoverShape = findShapeAtPosition(event->point);
        int hoverHandle = -1;
        if (hoverShape) {
            KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(hoverShape);
            if (!connectionShape) {
                QPointF snappedPos = canvas()->snapGuide()->snap(event->point, event->modifiers());
                hoverHandle = handleAtPoint(hoverShape, snappedPos);
                setEditMode(hoverHandle >= 0 ? CreateConnection : Idle, hoverShape, hoverHandle);
            }
            useCursor(hoverHandle >= 0 ? m_connectCursor : Qt::PointingHandCursor);
        } else {
            useCursor(Qt::ArrowCursor);
        }
    }
}

void ConnectionTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        if (m_editMode == CreateConnection) {
            // check if connection handles have a minimal distance
            KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(m_currentShape);
            Q_ASSERT(connectionShape);
            // get both handle positions in document coordinates
            QPointF p1 = connectionShape->shapeToDocument(connectionShape->handlePosition(0));
            QPointF p2 = connectionShape->shapeToDocument(connectionShape->handlePosition(1));
            int grabDistance = grabSensitivity();
            // use grabbing sensitivity as minimal distance threshold
            if (squareDistance(p1, p2) < grabDistance * grabDistance) {
                // minimal distance was not reached, so we have to undo the started work:
                // - cleanup and delete the strategy
                // - remove connection shape from shape manager and delete it
                // - reset edit mode to last state
                delete m_currentStrategy;
                m_currentStrategy = 0;
                repaintDecorations();
                canvas()->shapeManager()->remove(m_currentShape);
                setEditMode(m_editMode, connectionShape->firstShape(), connectionShape->firstConnectionId());
                repaintDecorations();
                delete connectionShape;
                return;
            } else {
                // finalize adding the new connection shape with an undo command
                KUndo2Command *cmd = canvas()->shapeController()->addShape(m_currentShape, 0);
                canvas()->addCommand(cmd);
                setEditMode(EditConnection, m_currentShape, KoConnectionShape::StartHandle);
            }
        }
        m_currentStrategy->finishInteraction(event->modifiers());
        // TODO: Add parent command to KoInteractionStrategy::createCommand
        // so that we can have a single command to undo for the user
        KUndo2Command *command = m_currentStrategy->createCommand();
        if (command) {
            canvas()->addCommand(command);
        }
        delete m_currentStrategy;
        m_currentStrategy = 0;
    }
    updateStatusText();
}

void ConnectionTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (m_editMode == EditConnectionPoint) {
        repaintDecorations();

        //quit EditConnectionPoint mode when double click blank region on canvas
        if (!m_currentShape) {
            resetEditMode();
            return;
        }

        //add connection point when double click a shape
        //remove connection point when double click a existed connection point
        int handleId = handleAtPoint(m_currentShape, event->point);
        if (handleId < 0) {
            QPointF mousePos = canvas()->snapGuide()->snap(event->point, event->modifiers());
            QPointF point = m_currentShape->documentToShape(mousePos);
            canvas()->addCommand(new AddConnectionPointCommand(m_currentShape, point));
        } else {
            canvas()->addCommand(new RemoveConnectionPointCommand(m_currentShape, handleId));
        }
        setEditMode(m_editMode, m_currentShape, -1);
    } else {
        //deactivate connection tool when double click blank region on canvas
        KoShape *hitShape = findShapeAtPosition(event->point);
        if (!hitShape) {
            deactivate();
            emit done();
        } else if (dynamic_cast<KoConnectionShape *>(hitShape)) {
            repaintDecorations();
            setEditMode(EditConnection, m_currentShape, -1);
            //TODO: temporarily activate text tool to edit connection path
        }
    }
}

void ConnectionTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        deactivate();
        emit done();
    } else if (event->key() == Qt::Key_Backspace) {
        deleteSelection();
        event->accept();
    }
}

void ConnectionTool::activate(ToolActivation activation, const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(activation, shapes);

    // save old enabled snap strategies, set bounding box snap strategy
    m_oldSnapStrategies = canvas()->snapGuide()->enabledSnapStrategies();
    canvas()->snapGuide()->enableSnapStrategies(KoSnapGuide::BoundingBoxSnapping);
    canvas()->snapGuide()->reset();
    m_resetPaint = true;
    repaintDecorations();
}

void ConnectionTool::deactivate()
{
    // Put everything to 0 to be able to begin a new shape properly
    delete m_currentStrategy;
    m_currentStrategy = 0;
    resetEditMode();
    m_resetPaint = true;
    repaintDecorations();
    // restore previously set snap strategies
    canvas()->snapGuide()->enableSnapStrategies(m_oldSnapStrategies);
    canvas()->snapGuide()->reset();
    KoToolBase::deactivate();
}

qreal ConnectionTool::squareDistance(const QPointF &p1, const QPointF &p2) const
{
    // Square of the distance
    const qreal dx = p2.x() - p1.x();
    const qreal dy = p2.y() - p1.y();
    return dx * dx + dy * dy;
}

KoShape *ConnectionTool::findShapeAtPosition(const QPointF &position) const
{
    QList<KoShape *> shapes = canvas()->shapeManager()->shapesAt(handleGrabRect(position));
    if (!shapes.isEmpty()) {
        std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        // we want to priorize connection shape handles, even if the connection shape
        // is not at the top of the shape stack at the mouse position
        KoConnectionShape *connectionShape = nearestConnectionShape(shapes, position);
        // use best connection shape or first shape from stack (last in the list) if not found
        if (connectionShape) {
            return connectionShape;
        } else {
            for (QList<KoShape *>::const_iterator end = shapes.constEnd() - 1; end >= shapes.constBegin(); --end) {
                KoShape *shape = *end;
                if (!dynamic_cast<KoConnectionShape *>(shape) && shape->shapeId() != TextShape_SHAPEID) {
                    return shape;
                }
            }
        }
    }

    return 0;
}

KoShape *ConnectionTool::findNonConnectionShapeAtPosition(const QPointF &position) const
{
    QList<KoShape *> shapes = canvas()->shapeManager()->shapesAt(handleGrabRect(position));
    if (!shapes.isEmpty()) {
        std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        for (QList<KoShape *>::const_iterator end = shapes.constEnd() - 1; end >= shapes.constBegin(); --end) {
            KoShape *shape = *end;
            if (!dynamic_cast<KoConnectionShape *>(shape) && shape->shapeId() != TextShape_SHAPEID) {
                return shape;
            }
        }
    }

    return 0;
}

int ConnectionTool::handleAtPoint(KoShape *shape, const QPointF &mousePoint) const
{
    if (!shape) {
        return -1;
    }

    const QPointF shapePoint = shape->documentToShape(mousePoint);

    KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(shape);
    if (connectionShape) {
        // check connection shape handles
        return connectionShape->handleIdAt(handleGrabRect(shapePoint));
    } else {
        // check connection points
        int grabDistance = grabSensitivity();
        qreal minDistance = HUGE_VAL;
        int handleId = -1;
        KoConnectionPoints connectionPoints = shape->connectionPoints();
        KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
        KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
        for (; cp != lastCp; ++cp) {
            qreal d = squareDistance(shapePoint, cp.value().position);
            if (d <= grabDistance && d < minDistance) {
                handleId = cp.key();
                minDistance = d;
            }
        }
        return handleId;
    }
}

KoConnectionShape *ConnectionTool::nearestConnectionShape(const QList<KoShape *> &shapes, const QPointF &mousePos) const
{
    int grabDistance = grabSensitivity();

    KoConnectionShape *nearestConnectionShape = 0;
    qreal minSquaredDistance = HUGE_VAL;
    const qreal maxSquaredDistance = grabDistance * grabDistance;

    Q_FOREACH (KoShape *shape, shapes) {
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(shape);
        if (!connectionShape || !connectionShape->isParametricShape()) {
            continue;
        }

        // convert document point to shape coordinates
        QPointF p = connectionShape->documentToShape(mousePos);
        // our region of interest, i.e. a region around our mouse position
        QRectF roi = handleGrabRect(p);

        // check all segments of this shape which intersect the region of interest
        QList<KoPathSegment> segments = connectionShape->segmentsAt(roi);
        foreach (const KoPathSegment &s, segments) {
            qreal nearestPointParam = s.nearestPoint(p);
            QPointF nearestPoint = s.pointAt(nearestPointParam);
            QPointF diff = p - nearestPoint;
            qreal squaredDistance = diff.x() * diff.x() + diff.y() * diff.y();
            // are we within the allowed distance ?
            if (squaredDistance > maxSquaredDistance) {
                continue;
            }
            // are we closer to the last closest point ?
            if (squaredDistance < minSquaredDistance) {
                nearestConnectionShape = connectionShape;
                minSquaredDistance = squaredDistance;
            }
        }
    }

    return nearestConnectionShape;
}

void ConnectionTool::setEditMode(EditMode mode, KoShape *currentShape, int handle)
{
    repaintDecorations();
    m_editMode = mode;
    if (m_currentShape != currentShape) {
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(currentShape);
        foreach (KoShapeConfigWidgetBase *cw, m_connectionShapeWidgets) {
            if (connectionShape) {
                cw->open(currentShape);
            }
        }
    }
    if (mode == Idle) {
        emit sendConnectionType(m_connectionType);
    }
    m_currentShape = currentShape;
    m_activeHandle = handle;
    repaintDecorations();
    updateActions();
    updateStatusText();
}

void ConnectionTool::resetEditMode()
{
    m_connectionType = KoConnectionShape::Standard;
    setEditMode(Idle, 0, -1);
    emit sendConnectionPointEditState(false);
}

void ConnectionTool::updateActions()
{
    const bool connectionPointSelected = m_editMode == EditConnectionPoint && m_activeHandle >= 0;
    if (connectionPointSelected) {
        KoConnectionPoint cp = m_currentShape->connectionPoint(m_activeHandle);

        m_alignPercent->setChecked(false);
        Q_FOREACH (QAction *action, m_alignHorizontal->actions()) {
            action->setChecked(false);
        }
        Q_FOREACH (QAction *action, m_alignVertical->actions()) {
            action->setChecked(false);
        }
        switch (cp.alignment) {
        case KoConnectionPoint::AlignNone:
            m_alignPercent->setChecked(true);
            break;
        case KoConnectionPoint::AlignTopLeft:
            m_alignLeft->setChecked(true);
            m_alignTop->setChecked(true);
            break;
        case KoConnectionPoint::AlignTop:
            m_alignCenterH->setChecked(true);
            m_alignTop->setChecked(true);
            break;
        case KoConnectionPoint::AlignTopRight:
            m_alignRight->setChecked(true);
            m_alignTop->setChecked(true);
            break;
        case KoConnectionPoint::AlignLeft:
            m_alignLeft->setChecked(true);
            m_alignCenterV->setChecked(true);
            break;
        case KoConnectionPoint::AlignCenter:
            m_alignCenterH->setChecked(true);
            m_alignCenterV->setChecked(true);
            break;
        case KoConnectionPoint::AlignRight:
            m_alignRight->setChecked(true);
            m_alignCenterV->setChecked(true);
            break;
        case KoConnectionPoint::AlignBottomLeft:
            m_alignLeft->setChecked(true);
            m_alignBottom->setChecked(true);
            break;
        case KoConnectionPoint::AlignBottom:
            m_alignCenterH->setChecked(true);
            m_alignBottom->setChecked(true);
            break;
        case KoConnectionPoint::AlignBottomRight:
            m_alignRight->setChecked(true);
            m_alignBottom->setChecked(true);
            break;
        }
        Q_FOREACH (QAction *action, m_escapeDirections->actions()) {
            action->setChecked(false);
        }
        switch (cp.escapeDirection) {
        case KoConnectionPoint::AllDirections:
            m_escapeAll->setChecked(true);
            break;
        case KoConnectionPoint::HorizontalDirections:
            m_escapeHorizontal->setChecked(true);
            break;
        case KoConnectionPoint::VerticalDirections:
            m_escapeVertical->setChecked(true);
            break;
        case KoConnectionPoint::LeftDirection:
            m_escapeLeft->setChecked(true);
            break;
        case KoConnectionPoint::RightDirection:
            m_escapeRight->setChecked(true);
            break;
        case KoConnectionPoint::UpDirection:
            m_escapeUp->setChecked(true);
            break;
        case KoConnectionPoint::DownDirection:
            m_escapeDown->setChecked(true);
            break;
        }
    }
    emit connectionPointEnabled(connectionPointSelected);
}

void ConnectionTool::updateStatusText()
{
    switch (m_editMode) {
    case Idle:
        if (m_currentShape) {
            if (dynamic_cast<KoConnectionShape *>(m_currentShape)) {
                if (m_activeHandle >= 0) {
                    emit statusTextChanged(i18n("Drag to edit connection."));
                } else {
                    emit statusTextChanged(i18n("Double click connection or press delete to remove it."));
                }
            } else if (m_activeHandle < 0) {
                emit statusTextChanged(i18n("Click to edit connection points."));
            }
        } else {
            emit statusTextChanged(QString());
        }
        break;
    case EditConnection:
        if (m_activeHandle >= 0) {
            emit statusTextChanged(i18n("Drag to edit connection."));
        } else {
            emit statusTextChanged(i18n("Double click connection or press delete to remove it."));
        }
        break;
    case EditConnectionPoint:
        if (m_activeHandle >= KoConnectionPoint::FirstCustomConnectionPoint) {
            emit statusTextChanged(i18n("Drag to move connection point. Double click connection or press delete to remove it."));
        } else if (m_activeHandle >= 0) {
            emit statusTextChanged(i18n("Double click connection point or press delete to remove it."));
        } else {
            emit statusTextChanged(i18n("Double click to add connection point."));
        }
        break;
    case CreateConnection:
        emit statusTextChanged(i18n("Drag to create new connection."));
        break;
    default:
        emit statusTextChanged(QString());
    }
}

QList<QPointer<QWidget> > ConnectionTool::createOptionWidgets()
{
    QList<QPointer<QWidget> > list;

    m_connectionShapeWidgets.clear();

    KoShapeFactoryBase *factory = KoShapeRegistry::instance()->get(KOCONNECTIONSHAPEID);
    if (factory) {
        QList<KoShapeConfigWidgetBase *> widgets = factory->createShapeOptionPanels();
        Q_FOREACH (KoShapeConfigWidgetBase *cw, widgets) {
            if (cw->showOnShapeCreate() || !cw->showOnShapeSelect()) {
                delete cw;
                continue;
            }
            connect(cw, SIGNAL(propertyChanged()), this, SLOT(connectionChanged()));
            KoConnectionShapeConfigWidget *cw2 = (KoConnectionShapeConfigWidget *)cw;
            if (cw2) {
                connect(cw2, SIGNAL(connectionTypeChanged(int)), this, SLOT(getConnectionType(int)));
                connect(this, SIGNAL(sendConnectionType(int)), cw2, SLOT(setConnectionType(int)));
            }
            m_connectionShapeWidgets.append(cw);
            cw->setWindowTitle(i18n("Connection"));
            list.append(cw);
        }
    }

    KoStrokeConfigWidget *strokeWidget = new KoStrokeConfigWidget(canvas(), 0);
    KisDocumentAwareSpinBoxUnitManager* managerLineWidth = new KisDocumentAwareSpinBoxUnitManager(strokeWidget);
    KisDocumentAwareSpinBoxUnitManager* managerMitterLimit = new KisDocumentAwareSpinBoxUnitManager(strokeWidget);
    managerLineWidth->setApparentUnitFromSymbol("px");
    managerMitterLimit->setApparentUnitFromSymbol("px");
    strokeWidget->setUnitManagers(managerLineWidth, managerMitterLimit);
    strokeWidget->setWindowTitle(i18n("Line"));
    list.append(strokeWidget);

    ConnectionPointWidget *connectPoint = new ConnectionPointWidget(this);
    connectPoint->setWindowTitle(i18n("Connection Point"));
    list.append(connectPoint);

    return list;
}

void ConnectionTool::horizontalAlignChanged()
{
    if (m_alignPercent->isChecked()) {
        m_alignPercent->setChecked(false);
        m_alignTop->setChecked(true);
    }
    updateConnectionPoint();
}

void ConnectionTool::verticalAlignChanged()
{
    if (m_alignPercent->isChecked()) {
        m_alignPercent->setChecked(false);
        m_alignLeft->setChecked(true);
    }
    updateConnectionPoint();
}

void ConnectionTool::relativeAlignChanged()
{
    Q_FOREACH (QAction *action, m_alignHorizontal->actions()) {
        action->setChecked(false);
    }
    Q_FOREACH (QAction *action, m_alignVertical->actions()) {
        action->setChecked(false);
    }
    m_alignPercent->setChecked(true);

    updateConnectionPoint();
}

void ConnectionTool::updateConnectionPoint()
{
    if (m_editMode == EditConnectionPoint && m_currentShape && m_activeHandle >= 0) {
        KoConnectionPoint oldPoint = m_currentShape->connectionPoint(m_activeHandle);
        KoConnectionPoint newPoint = oldPoint;
        if (m_alignPercent->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignNone;
        } else if (m_alignLeft->isChecked() && m_alignTop->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignTopLeft;
        } else if (m_alignCenterH->isChecked() && m_alignTop->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignTop;
        } else if (m_alignRight->isChecked() && m_alignTop->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignTopRight;
        } else if (m_alignLeft->isChecked() && m_alignCenterV->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignLeft;
        } else if (m_alignCenterH->isChecked() && m_alignCenterV->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignCenter;
        } else if (m_alignRight->isChecked() && m_alignCenterV->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignRight;
        } else if (m_alignLeft->isChecked() && m_alignBottom->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignBottomLeft;
        } else if (m_alignCenterH->isChecked() && m_alignBottom->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignBottom;
        } else if (m_alignRight->isChecked() && m_alignBottom->isChecked()) {
            newPoint.alignment = KoConnectionPoint::AlignBottomRight;
        }

        canvas()->addCommand(new ChangeConnectionPointCommand(m_currentShape, m_activeHandle, oldPoint, newPoint));
    }
}

void ConnectionTool::escapeDirectionChanged()
{
    if (m_editMode == EditConnectionPoint && m_currentShape && m_activeHandle >= 0) {
        KoConnectionPoint oldPoint = m_currentShape->connectionPoint(m_activeHandle);
        KoConnectionPoint newPoint = oldPoint;
        QAction *checkedAction = m_escapeDirections->checkedAction();
        if (checkedAction == m_escapeAll) {
            newPoint.escapeDirection = KoConnectionPoint::AllDirections;
        } else if (checkedAction == m_escapeHorizontal) {
            newPoint.escapeDirection = KoConnectionPoint::HorizontalDirections;
        } else if (checkedAction == m_escapeVertical) {
            newPoint.escapeDirection = KoConnectionPoint::VerticalDirections;
        } else if (checkedAction == m_escapeLeft) {
            newPoint.escapeDirection = KoConnectionPoint::LeftDirection;
        } else if (checkedAction == m_escapeRight) {
            newPoint.escapeDirection = KoConnectionPoint::RightDirection;
        } else if (checkedAction == m_escapeUp) {
            newPoint.escapeDirection = KoConnectionPoint::UpDirection;
        } else if (checkedAction == m_escapeDown) {
            newPoint.escapeDirection = KoConnectionPoint::DownDirection;
        }
        canvas()->addCommand(new ChangeConnectionPointCommand(m_currentShape, m_activeHandle, oldPoint, newPoint));
    }
}

void ConnectionTool::connectionChanged()
{
    if (m_editMode != EditConnection) {
        return;
    }
    KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape *>(m_currentShape);
    if (!connectionShape) {
        return;
    }

    Q_FOREACH (KoShapeConfigWidgetBase *cw, m_connectionShapeWidgets) {
        canvas()->addCommand(cw->createCommand());
    }
}

void ConnectionTool::deleteSelection()
{
    if (m_editMode == EditConnectionPoint && m_currentShape && m_activeHandle >= 0) {
        repaintDecorations();
        canvas()->addCommand(new RemoveConnectionPointCommand(m_currentShape, m_activeHandle));
        setEditMode(m_editMode, m_currentShape, -1);
    } else if (m_editMode == EditConnection && m_currentShape) {
        repaintDecorations();
        canvas()->addCommand(canvas()->shapeController()->removeShape(m_currentShape));
        resetEditMode();
    }
}

void ConnectionTool::getConnectionType(int type)
{
    if (m_editMode == Idle) {
        m_connectionType = (KoConnectionShape::Type)type;
    }
}

void ConnectionTool::toggleConnectionPointEditMode(int state)
{
    if (state == Qt::Checked) {
        setEditMode(EditConnectionPoint, 0, -1);
    } else if (state == Qt::Unchecked) {
        setEditMode(Idle, 0, -1);
    } else {
        return;
    }
}
