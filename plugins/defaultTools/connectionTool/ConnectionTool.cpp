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
#include <KLocale>
#include <KDebug>
#include <QUndoCommand>
#include <QPointF>
#include <QKeyEvent>

/*
 * TODO: use strategies to handle different tool actions:
 * 1. edit connections (reuse KoPathTools connection strategy) DONE
 * 2. create new connections                                   DONE
 * 3. remove connections                                       DONE
 * 4. edit connection points of a shape
 *    -> click on a shape to activate editing                  DONE
 *    - add connection points                                  DONE
 *    - remove connection points                               DONE
 *    - move connection points
 */

ConnectionTool::ConnectionTool(KoCanvasBase * canvas)
    : KoPathTool(canvas)
    , m_editMode(Idle)
    , m_shapeOn(0)
    , m_activeHandle(-1)
    , m_currentStrategy(0)
{
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

    if(m_shapeOn) {
        // paint connection points or connection handles depending
        // on the shape the mouse is currently
        KoConnectionShape *connectionShape = dynamic_cast<KoConnectionShape*>(m_shapeOn);
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
            QTransform transform = m_shapeOn->absoluteTransformation(0);
            KoShape::applyConversion(painter, converter);
            // Draw all the connection points of the shape
            KoConnectionPoints connectionPoints = m_shapeOn->connectionPoints();
            KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
            KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
            for(; cp != lastCp; ++cp) {
                handleRect.moveCenter(transform.map(cp.value()));
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
    if(m_shapeOn) {
        repaint(m_shapeOn->boundingRect());
        KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>(m_shapeOn);
        if(connectionShape) {
            QPointF handlePos = connectionShape->handlePosition(m_activeHandle);
            handlePos = connectionShape->shapeToDocument(handlePos);
            repaint(handlePaintRect(handlePos));
        } else {
            QRectF bbox;
            KoConnectionPoints connectionPoints = m_shapeOn->connectionPoints();
            KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
            KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
            for(; cp != lastCp; ++cp) {
                bbox.unite(handleGrabRect(m_shapeOn->shapeToDocument(cp.value())));
            }
            repaint(bbox);
        }
    }
}

void ConnectionTool::mousePressEvent(KoPointerEvent * event)
{
    if (m_editMode == Idle) {
        // pressing aon a shape in idle mode switches to connection point edit mode
        if (m_shapeOn && !dynamic_cast<KoConnectionShape*>(m_shapeOn)) {
            m_editMode = EditConnectionPoint;
            m_activeHandle = -1;
            repaintDecorations();
        }
    } else if(m_editMode == EditConnection) {
        // create connection handle change strategy
        m_currentStrategy = createStrategy(dynamic_cast<KoConnectionShape*>(m_shapeOn), m_activeHandle);
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
        QPointF cp = m_shapeOn->shapeToDocument(m_shapeOn->connectionPoint(m_activeHandle));
        // move both handles to that point
        connectionShape->moveHandle(0, cp);
        connectionShape->moveHandle(1, cp);
        // connect the first handle of the connection shape to our connection point
        if(!connectionShape->connectFirst(m_shapeOn, m_activeHandle)) {
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
        m_activeHandle = 1;
        m_shapeOn = shape;
        // add connection shape to the shape manager so it gets painted
        canvas()->shapeManager()->addShape(connectionShape);
    } else if (m_editMode == EditConnectionPoint) {
        // clicking not on a connection point and not on the current shape exits
        // connection point edit mode
        if (m_activeHandle < 0) {
            if (!canvas()->shapeManager()->shapesAt(handleGrabRect(event->point)).contains(m_shapeOn)) {
                repaintDecorations();
                resetEditMode();
            }
        }
    }
}

void ConnectionTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        repaintDecorations();
        m_currentStrategy->handleMouseMove(event->point, event->modifiers());
        repaintDecorations();
        return;
    }

    repaintDecorations();

    if (m_editMode == EditConnectionPoint) {
        Q_ASSERT(m_shapeOn);
        // check if we should highlight another connection point
        int handleId = handleAtPoint(m_shapeOn, event->point);
        if(handleId != m_activeHandle) {
            m_activeHandle = handleId;
            repaintDecorations();
        }
    } else {
        resetEditMode();

        QList<KoShape*> shapes = canvas()->shapeManager()->shapesAt(handleGrabRect(event->point));
        if(!shapes.isEmpty()) {
            qSort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
            // we want to priorize connection shape handles, even if the connection shape
            // is not at the top of the shape stack at the mouse position
            KoConnectionShape *connectionShape = nearestConnectionShape(shapes, event->point);
            // use best connection shape or first shape from stack if not found
            m_shapeOn = connectionShape ? connectionShape : shapes.first();
            int handleId = handleAtPoint(m_shapeOn, event->point);
            if(handleId >= 0) {
                m_activeHandle = handleId;
                m_editMode = connectionShape ? EditConnection : CreateConnection;
            }
        }
    }

    switch(m_editMode) {
        case Idle:
            if(m_shapeOn) {
                if (dynamic_cast<KoConnectionShape*>(m_shapeOn))
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
            if (m_activeHandle >= 0)
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

    repaintDecorations();
}

void ConnectionTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (m_currentStrategy) {
        if (m_editMode == CreateConnection) {
            // check if connection handles have a minimal distance
            KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>(m_shapeOn);
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
                canvas()->shapeManager()->remove(m_shapeOn);
                m_shapeOn = connectionShape->firstShape();
                m_activeHandle = connectionShape->firstConnectionId();
                repaintDecorations();
                delete connectionShape;
                return;
            } else {
                // finalize adding the new connection shape with an undo command
                QUndoCommand * cmd = canvas()->shapeController()->addShape(m_shapeOn);
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
        resetEditMode();
    }
}

void ConnectionTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (!m_shapeOn)
        return;

    if (m_editMode == EditConnectionPoint) {
        repaintDecorations();
        int handleId = handleAtPoint(m_shapeOn, event->point);
        if (handleId < 0) {
            QPointF point = m_shapeOn->documentToShape(event->point);
            // TODO: use an undo command
            m_shapeOn->addConnectionPoint(point);
        } else {
            // TODO: use an undo command
            m_shapeOn->removeConnectionPoint(handleId);
        }
        repaintDecorations();
        m_activeHandle = -1;
    } else if (m_editMode == Idle) {
        if (dynamic_cast<KoConnectionShape*>(m_shapeOn)) {
            repaintDecorations();
            QUndoCommand * cmd = canvas()->shapeController()->removeShape(m_shapeOn);
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
}

void ConnectionTool::deactivate()
{
    // Put everything to 0 to be able to begin a new shape properly
    delete m_currentStrategy;
    m_currentStrategy = 0;
    resetEditMode();
}

qreal ConnectionTool::squareDistance(const QPointF &p1, const QPointF &p2)
{
    // Square of the distance
    const qreal dx = p2.x() - p1.x();
    const qreal dy = p2.y() - p1.y();
    return dx*dx + dy*dy;
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
            qreal d = squareDistance(shapePoint, cp.value());
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
    qreal minSqaredDistance = HUGE_VAL;
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
            if (squaredDistance < minSqaredDistance) {
                nearestConnectionShape = connectionShape;
            }
        }
    }

    return nearestConnectionShape;
}

void ConnectionTool::resetEditMode()
{
    m_editMode = Idle;
    m_shapeOn = 0;
    m_activeHandle = -1;
    emit statusTextChanged("");
}
