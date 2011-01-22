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
#include "AddConnectionPointCommand.h"
#include "RemoveConnectionPointCommand.h"
#include "MoveConnectionPointStrategy.h"
#include "ConnectionToolWidget.h"
#include "ConnectionPointWidget.h"

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoShapeFactoryBase.h>
#include <KoShape.h>
#include <KoShapeController.h>
#include <KoShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoSelection.h>
#include <KoLineBorder.h>
#include <KoResourceManager.h>
#include <KoInteractionStrategy.h>
#include <KAction>
#include <KLocale>
#include <KDebug>
#include <QUndoCommand>
#include <QPointF>
#include <QKeyEvent>

ConnectionTool::ConnectionTool(KoCanvasBase * canvas)
    : KoPathTool(canvas)
    , m_editMode(Idle)
    , m_currentShape(0)
    , m_activeHandle(-1)
    , m_currentStrategy(0)
    , m_oldSnapStrategies(0)
{
    m_alignPercent = new KAction(QString("%"), this);
    m_alignPercent->setCheckable(true);
    addAction("align-relative", m_alignPercent);
    m_alignLeft = new KAction(KIcon("align-horizontal-left"), i18n("Align to left edge"), this);
    m_alignLeft->setCheckable(true);
    addAction("align-left", m_alignLeft);
    m_alignCenterH = new KAction(KIcon("align-horizontal-center"), i18n("Align to horizontal center"), this);
    m_alignCenterH->setCheckable(true);
    addAction("align-centerh", m_alignCenterH);
    m_alignRight = new KAction(KIcon("align-horizontal-right"), i18n("Align to right edge"), this);
    m_alignRight->setCheckable(true);
    addAction("align-right", m_alignRight);
    m_alignTop = new KAction(KIcon("align-vertical-top"), i18n("Align to top edge"), this);
    m_alignTop->setCheckable(true);
    addAction("align-top", m_alignTop);
    m_alignCenterV = new KAction(KIcon("align-vertical-center"), i18n("Align to vertical center"), this);
    m_alignCenterV->setCheckable(true);
    addAction("align-centerv", m_alignCenterV);
    m_alignBottom = new KAction(KIcon("align-vertical-bottom"), i18n("Align to bottom edge"), this);
    m_alignBottom->setCheckable(true);
    addAction("align-bottom", m_alignBottom);

    QActionGroup *alignHorizontal = new QActionGroup(this);
    alignHorizontal->setExclusive(true);
    alignHorizontal->addAction(m_alignLeft);
    alignHorizontal->addAction(m_alignCenterH);
    alignHorizontal->addAction(m_alignRight);
    connect(alignHorizontal, SIGNAL(triggered(QAction*)), this, SLOT(horizontalAlignChanged()));

    QActionGroup *alignVertical = new QActionGroup(this);
    alignVertical->setExclusive(true);
    alignVertical->addAction(m_alignTop);
    alignVertical->addAction(m_alignCenterV);
    alignVertical->addAction(m_alignBottom);
    connect(alignVertical, SIGNAL(triggered(QAction*)), this, SLOT(verticalAlignChanged()));

    QActionGroup *alignRelative = new QActionGroup(this);
    alignRelative->setExclusive(true);
    alignRelative->addAction(m_alignPercent);
    connect(alignRelative, SIGNAL(triggered(QAction*)), this, SLOT(relativeAlignChanged()));

    connect(this, SIGNAL(connectionPointEnabled(bool)), alignHorizontal, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(connectionPointEnabled(bool)), alignVertical, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(connectionPointEnabled(bool)), alignRelative, SLOT(setEnabled(bool)));

    setEditMode(Idle, 0, -1);
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

    if(m_currentShape) {
        // paint connection points or connection handles depending
        // on the shape the mouse is currently
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape*>(m_currentShape);
        if(connectionShape) {
            int radius = canvas()->resourceManager()->handleRadius();
            int handleCount = connectionShape->handleCount();
            for(int i = 0; i < handleCount; ++i) {
                painter.save();
                painter.setPen(Qt::blue);
                painter.setBrush(i == m_activeHandle ? Qt::red : Qt::white);
                painter.setTransform(connectionShape->absoluteTransformation(&converter) * painter.transform());
                connectionShape->paintHandle(painter, converter, i, radius);
                painter.restore();
            }
        } else {
            painter.save();
            painter.setPen(Qt::black);
            QTransform transform = m_currentShape->absoluteTransformation(0);
            KoShape::applyConversion(painter, converter);
            // Draw all the connection points of the shape
            KoConnectionPoints connectionPoints = m_currentShape->connectionPoints();
            KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
            KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
            for(; cp != lastCp; ++cp) {
                handleRect.moveCenter(transform.map(cp.value().position));
                Qt::GlobalColor fillColor = Qt::white;
                if(m_editMode == EditConnectionPoint) {
                    fillColor = cp.key() == m_activeHandle ? Qt::red : Qt::darkGreen;
                } else if (m_editMode == CreateConnection) {
                    fillColor = cp.key() == m_activeHandle ? Qt::red : Qt::white;
                }
                painter.setBrush(fillColor);
                painter.drawRect(handleRect);
            }
            painter.restore();
        }
    }
}

void ConnectionTool::repaintDecorations()
{
    if(m_currentShape) {
        repaint(m_currentShape->boundingRect());
        KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>(m_currentShape);
        if(connectionShape) {
            QPointF handlePos = connectionShape->handlePosition(m_activeHandle);
            handlePos = connectionShape->shapeToDocument(handlePos);
            repaint(handlePaintRect(handlePos));
        } else {
            KoConnectionPoints connectionPoints = m_currentShape->connectionPoints();
            KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
            KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
            for(; cp != lastCp; ++cp) {
                repaint(handleGrabRect(m_currentShape->shapeToDocument(cp.value().position)));
            }
        }
    }
}

void ConnectionTool::mousePressEvent(KoPointerEvent * event)
{
    if (m_editMode == Idle) {
        // pressing on a shape in idle mode switches to connection point edit mode
        if (m_currentShape && !dynamic_cast<KoConnectionShape*>(m_currentShape)) {
            m_editMode = EditConnectionPoint;
            m_activeHandle = -1;
            repaintDecorations();
        }
    } else if(m_editMode == EditConnection) {
        // create connection handle change strategy
        m_currentStrategy = createStrategy(dynamic_cast<KoConnectionShape*>(m_currentShape), m_activeHandle);
    } else if(m_editMode == CreateConnection) {
        // create new connection shape, connect it to the active connection point
        // and start editing the new connection
        repaintDecorations();
        // create the new connection shape
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
        KoShape *shape = factory->createDefaultShape(canvas()->shapeController()->resourceManager());
        KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>(shape);
        if(!connectionShape) {
            delete shape;
            resetEditMode();
            return;
        }
        // get the position of the connection point we start our connection from
        QPointF cp = m_currentShape->shapeToDocument(m_currentShape->connectionPoint(m_activeHandle).position);
        // move both handles to that point
        connectionShape->moveHandle(0, cp);
        connectionShape->moveHandle(1, cp);
        // connect the first handle of the connection shape to our connection point
        if(!connectionShape->connectFirst(m_currentShape, m_activeHandle)) {
            delete shape;
            resetEditMode();
            return;
        }
        // create the connection edit strategy from the path tool
        m_currentStrategy = createStrategy(connectionShape, 1);
        if (!m_currentStrategy) {
            delete shape;
            resetEditMode();
            return;
        }
        // update our handle data
        setEditMode(m_editMode, shape, 1);
        // add connection shape to the shape manager so it gets painted
        canvas()->shapeManager()->addShape(connectionShape);
    } else if (m_editMode == EditConnectionPoint) {
        // clicking not on a connection point and not on the current shape exits
        // connection point edit mode
        int handle = handleAtPoint(m_currentShape, event->point);
        if (handle < 0) {
            // check we are on the current shape
            if (!canvas()->shapeManager()->shapesAt(handleGrabRect(event->point)).contains(m_currentShape)) {
                repaintDecorations();
                findShapeAtPosition(event->point);
                if (m_currentShape && !dynamic_cast<KoConnectionShape*>(m_currentShape)) {
                    setEditMode(EditConnectionPoint, m_currentShape, -1);
                    repaintDecorations();
                }
            }
        } else {
            if (m_activeHandle >= KoConnectionPoint::FirstCustomConnectionPoint) {
                m_currentStrategy = new MoveConnectionPointStrategy(m_currentShape, m_activeHandle, this);
            }
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
        return;
    }

    repaintDecorations();

    if (m_editMode == EditConnectionPoint) {
        Q_ASSERT(m_currentShape);
        // check if we should highlight another connection point
        int handle = handleAtPoint(m_currentShape, event->point);
        if (handle >= 0)
            setEditMode(m_editMode, m_currentShape, handle);
    } else {
        findShapeAtPosition(event->point);
    }

    updateStatusText();
    repaintDecorations();
}

void ConnectionTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        if (m_editMode == CreateConnection) {
            // check if connection handles have a minimal distance
            KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>(m_currentShape);
            Q_ASSERT(connectionShape);
            // get both handle positions in document coordinates
            QPointF p1 = connectionShape->shapeToDocument(connectionShape->handlePosition(0));
            QPointF p2 = connectionShape->shapeToDocument(connectionShape->handlePosition(1));
            int grabSensitivity = canvas()->resourceManager()->grabSensitivity();
            // use grabbing sensitivity as minimal distance threshold
            if (squareDistance(p1, p2) < grabSensitivity*grabSensitivity) {
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
                QUndoCommand * cmd = canvas()->shapeController()->addShape(m_currentShape);
                canvas()->addCommand(cmd);
            }
        }
        m_currentStrategy->finishInteraction(event->modifiers());
        // TODO: Add parent command to KoInteractionStrategy::createCommand
        // so that we can have a single command to undo for the user
        QUndoCommand *command = m_currentStrategy->createCommand();
        if (command)
            canvas()->addCommand(command);
        delete m_currentStrategy;
        m_currentStrategy = 0;
        if (m_editMode == EditConnection || m_editMode == CreateConnection)
            resetEditMode();
    }
    updateStatusText();
}

void ConnectionTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (!m_currentShape)
        return;

    if (m_editMode == EditConnectionPoint) {
        repaintDecorations();
        int handleId = handleAtPoint(m_currentShape, event->point);
        if (handleId < 0) {
            QPointF mousePos = canvas()->snapGuide()->snap(event->point, event->modifiers());
            QPointF point = m_currentShape->documentToShape(mousePos);
            canvas()->addCommand(new AddConnectionPointCommand(m_currentShape, point));
        } else {
            canvas()->addCommand(new RemoveConnectionPointCommand(m_currentShape, handleId));
        }
        repaintDecorations();
        setEditMode(m_editMode, m_currentShape, -1);
    } else if (m_editMode == Idle) {
        if (dynamic_cast<KoConnectionShape*>(m_currentShape)) {
            repaintDecorations();
            QUndoCommand * cmd = canvas()->shapeController()->removeShape(m_currentShape);
            canvas()->addCommand(cmd);
            resetEditMode();
            repaintDecorations();
        }
    }
}

void ConnectionTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        deactivate();
    }
}

void ConnectionTool::activate(ToolActivation, const QSet<KoShape*> &)
{
    canvas()->canvasWidget()->setCursor(Qt::PointingHandCursor);
    // save old enabled snap strategies, set bounding box snap strategy
    m_oldSnapStrategies = canvas()->snapGuide()->enabledSnapStrategies();
    canvas()->snapGuide()->enableSnapStrategies(KoSnapGuide::BoundingBoxSnapping);
    canvas()->snapGuide()->reset();
}

void ConnectionTool::deactivate()
{
    // Put everything to 0 to be able to begin a new shape properly
    delete m_currentStrategy;
    m_currentStrategy = 0;
    resetEditMode();
    // restore previously set snap strategies
    canvas()->snapGuide()->enableSnapStrategies(m_oldSnapStrategies);
    canvas()->snapGuide()->reset();
}

qreal ConnectionTool::squareDistance(const QPointF &p1, const QPointF &p2)
{
    // Square of the distance
    const qreal dx = p2.x() - p1.x();
    const qreal dy = p2.y() - p1.y();
    return dx*dx + dy*dy;
}

void ConnectionTool::findShapeAtPosition(const QPointF &position)
{
    resetEditMode();
    QList<KoShape*> shapes = canvas()->shapeManager()->shapesAt(handleGrabRect(position));
    if(!shapes.isEmpty()) {
        qSort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        // we want to priorize connection shape handles, even if the connection shape
        // is not at the top of the shape stack at the mouse position
        KoConnectionShape *connectionShape = nearestConnectionShape(shapes, position);
        // use best connection shape or first shape from stack if not found
        KoShape *currentShape = connectionShape ? connectionShape : shapes.first();
        int handleId = handleAtPoint(currentShape, position);
        EditMode mode = m_editMode;
        if(handleId >= 0) {
            mode = connectionShape ? EditConnection : CreateConnection;
        }
        setEditMode(mode, currentShape, handleId);
    }
}

int ConnectionTool::handleAtPoint(KoShape *shape, const QPointF &mousePoint)
{
    if(!shape)
        return -1;

    const QPointF shapePoint = shape->documentToShape(mousePoint);

    KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>(shape);
    if(connectionShape) {
        // check connection shape handles
        return connectionShape->handleIdAt(handleGrabRect(shapePoint));
    } else {
        // check connection points
        int grabSensitivity = canvas()->resourceManager()->grabSensitivity();
        qreal minDistance = HUGE_VAL;
        int handleId = -1;
        KoConnectionPoints connectionPoints = shape->connectionPoints();
        KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
        KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
        for(; cp != lastCp; ++cp) {
            qreal d = squareDistance(shapePoint, cp.value().position);
            if (d <= grabSensitivity && d < minDistance) {
                handleId = cp.key();
                minDistance = d;
            }
        }
        return handleId;
    }
}

KoConnectionShape * ConnectionTool::nearestConnectionShape(QList<KoShape*> shapes, const QPointF &mousePos)
{
    int grabSensitivity = canvas()->resourceManager()->grabSensitivity();

    KoConnectionShape * nearestConnectionShape = 0;
    qreal minSquaredDistance = HUGE_VAL;
    const qreal maxSquaredDistance = grabSensitivity*grabSensitivity;

    foreach(KoShape *shape, shapes) {
        KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>(shape);
        if (!connectionShape || !connectionShape->isParametricShape())
            continue;

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
            qreal squaredDistance = diff.x()*diff.x() + diff.y()*diff.y();
            // are we within the allowed distance ?
            if (squaredDistance > maxSquaredDistance)
                continue;
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
    m_editMode = mode;
    m_currentShape = currentShape;
    m_activeHandle = handle;

    const bool connectionPointSelected = m_editMode == EditConnectionPoint && m_activeHandle >= 0;
    if (connectionPointSelected) {
        KoConnectionPoint cp = m_currentShape->connectionPoint(m_activeHandle);
        m_alignPercent->setChecked(false);
        m_alignLeft->setChecked(false);
        m_alignCenterH->setChecked(false);
        m_alignRight->setChecked(false);
        m_alignTop->setChecked(false);
        m_alignCenterV->setChecked(false);
        m_alignBottom->setChecked(false);
        switch(cp.align) {
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
    }
    emit connectionPointEnabled(connectionPointSelected);
}

void ConnectionTool::resetEditMode()
{
    setEditMode(Idle, 0, -1);
    emit statusTextChanged("");
}

void ConnectionTool::updateStatusText()
{
    switch(m_editMode) {
        case Idle:
            if(m_currentShape) {
                if (dynamic_cast<KoConnectionShape*>(m_currentShape))
                    emit statusTextChanged(i18n("Double click to remove connection."));
                else if (m_activeHandle < 0)
                    emit statusTextChanged(i18n("Click to edit connection points."));
            } else {
                emit statusTextChanged("");
            }
            break;
        case EditConnection:
            emit statusTextChanged(i18n("Drag to edit connection."));
            break;
        case EditConnectionPoint:
            if (m_activeHandle >= KoConnectionPoint::FirstCustomConnectionPoint)
                emit statusTextChanged(i18n("Drag to move connection point. Double click to remove connection point"));
            else if (m_activeHandle >= 0)
                emit statusTextChanged(i18n("Double click to remove connection point"));
            else
                emit statusTextChanged(i18n("Double click to add connection point."));
            break;
        case CreateConnection:
            emit statusTextChanged(i18n("Drag to create new connection."));
            break;
        default:
            emit statusTextChanged("");
    }
}

QMap<QString, QWidget *> ConnectionTool::createOptionWidgets()
{
    QMap<QString, QWidget *> map;

    ConnectionToolWidget *tw = new ConnectionToolWidget();
    ConnectionPointWidget *pw = new ConnectionPointWidget(this);

    map.insert(i18n("Connection"), tw);
    map.insert(i18n("Connection Point"), pw);

    return map;
}

void ConnectionTool::horizontalAlignChanged()
{
    if (m_alignPercent->isChecked()) {
        m_alignPercent->setChecked(false);
        m_alignCenterV->setChecked(true);
    }
    // TODO: change connection point align here
}

void ConnectionTool::verticalAlignChanged()
{
    if (m_alignPercent->isChecked()) {
        m_alignPercent->setChecked(false);
        m_alignCenterH->setChecked(true);
    }
    // TODO: change connection point align here
}

void ConnectionTool::relativeAlignChanged()
{
    m_alignLeft->setChecked(false);
    m_alignCenterH->setChecked(false);
    m_alignRight->setChecked(false);
    m_alignTop->setChecked(false);
    m_alignCenterV->setChecked(false);
    m_alignBottom->setChecked(false);
    m_alignPercent->setChecked(true);
    // TODO: change connection point align here
}
