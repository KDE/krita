/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#include <QUndoCommand>
#include <QPointF>
#include <QKeyEvent>

ConnectionTool::ConnectionTool(KoCanvasBase * canvas)
    : KoPathTool(canvas)
    , m_shape1(0)
    , m_shapeOn(0)
    , m_lastShapeOn(0)
    , m_connectionShape(0)
    , m_lastConnectionShapeOn(0)
{
    // Initialize the isTied for first and second shape
    m_isTied = new QPair<bool, bool>(false,false);
    // Nothing is active or modify for now
    m_activeHandle = -1;
    m_firstHandleIndex = 0;
    m_modifyConnection = false;
}

ConnectionTool::~ConnectionTool()
{
    delete m_isTied;
}

void ConnectionTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    // get the correctly sized rect for painting handles
    QRectF handleRect = handlePaintRect(QPointF());

    painter.setRenderHint(QPainter::Antialiasing, true);
    // Green rects
    if(m_shapeOn !=  0) {
        // save the painter to restore it later
        painter.save();
        // Apply the conversion make by the matrix transformation
        QTransform transform = m_shapeOn->absoluteTransformation(0);
        KoShape::applyConversion(painter, converter);
        foreach(const QPointF &point, m_shapeOn->connectionPoints())
        { // Draw all the connection point of the shape
            handleRect.moveCenter(transform.map(point));
            painter.fillRect(handleRect, QColor(Qt::darkGreen));
        }
        painter.restore();
    }
    // Blue recs (when the mouse is on a a green rec)
    if(isInRoi()){
        // save the painter to restore it later
        painter.save();
        // Apply the conversion make by the matrix transformation
        QTransform transform = m_lastShapeOn->absoluteTransformation(0);
        KoShape::applyConversion(painter, converter);
        QPointF pointSelected = m_lastShapeOn->connectionPoints().value(getConnectionIndex(m_lastShapeOn, m_mouse));
        handleRect.moveCenter(transform.map(pointSelected));
        painter.fillRect(handleRect, QColor(Qt::blue));

        painter.restore();
    }
    
    // Draw handles ...
    KoConnectionShape * tempShape = dynamic_cast<KoConnectionShape*>(m_shapeOn);
    if(tempShape){
        painter.save();

        painter.setPen(Qt::blue);
        painter.setBrush(Qt::white);
        int radius = canvas()->resourceManager()->handleRadius();
        // Apply the conversion make by the matrix transformation
        painter.setTransform(tempShape->absoluteTransformation(&converter) * painter.transform());
        // ... handle unselected
        tempShape->paintHandles(painter, converter, radius);

        painter.restore();

        int grabSensitivity = canvas()->resourceManager()->grabSensitivity();
        QRectF rec(m_mouse.x()-grabSensitivity/2, m_mouse.y()-grabSensitivity/2, grabSensitivity, grabSensitivity);
        int handleId = tempShape->handleIdAt(tempShape->documentToShape(rec));

        if(handleId != -1) {
            painter.save();
            // ... handle selected
            painter.setPen(Qt::blue);
            painter.setBrush(Qt::red);
            // Apply the conversion make by the matrix transformation
            painter.setTransform(tempShape->absoluteTransformation(&converter) * painter.transform());
            tempShape->paintHandle(painter, converter, handleId, radius);

            painter.restore();
        }
    }
}

void ConnectionTool::mousePressEvent(KoPointerEvent *event)
{
    // If we are pressing down the control key, it should add handles, not connectionShape
    if(event->modifiers() & Qt::ControlModifier)
        return;
    // We here try to recover the the last shape in case of the handle point is out of the shape
    KoShape * tempShape = 0;
    if(isInRoi())
        tempShape = m_lastShapeOn;
    else
        tempShape = canvas()->shapeManager()->shapeAt(event->point);
    
    // We take care if the shape under the mouse is not another connection shape
    KoConnectionShape * tempConnectionShape = dynamic_cast<KoConnectionShape*>(m_shapeOn);
    if(tempConnectionShape && m_connectionShape == 0){
        // grabSensitivity is defined by the user
        int grabSensitivity = canvas()->resourceManager()->grabSensitivity();
        QRectF rec(m_mouse.x()-grabSensitivity/2, m_mouse.y()-grabSensitivity/2, grabSensitivity, grabSensitivity);
        m_activeHandle = tempConnectionShape->handleIdAt(tempShape->documentToShape(rec));
        
        m_lastConnectionShapeOn = tempConnectionShape;
        m_modifyConnection = true;
        return;
    } else
        tempShape = m_lastShapeOn;
    
    // First click
    if(m_connectionShape == 0) {
        // All sizes and positions are hardcoded for now
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
        KoShape *shape = factory->createDefaultShape(canvas()->shapeController()->resourceManager());
        if((m_connectionShape = dynamic_cast<KoConnectionShape*>(shape))){
            KoConnectionShape * connectionShapeTest = dynamic_cast<KoConnectionShape*>(tempShape);
            if(isInRoi()) {
                m_shape1 = tempShape;
                m_firstHandleIndex = getConnectionIndex(tempShape, m_mouse);
                
                m_connectionShape->connectFirst(m_shape1, m_firstHandleIndex);
                m_isTied->first = true;
            // If the shape selected is not the background
            // We take care if the working tempShape is not another connection shape
            }else if(tempShape != 0 && !connectionShapeTest) {
                m_shape1 = tempShape;
                m_firstHandleIndex = 0;
                m_connectionShape->connectFirst(m_shape1, m_firstHandleIndex);
                m_isTied->first = false;
            } else {
                m_shape1 = 0;
                m_firstHandleIndex = 0;
                m_connectionShape->moveHandle(0, event->point);
            }
            
            m_connectionShape->moveHandle(1, event->point);
            // The connection is now done, so update for apply
            m_connectionShape->updateConnections();
            canvas()->shapeManager()->addShape(m_connectionShape);
        }
    } else {
    // Second click
    
        if(m_shape1 != 0)
            m_connectionShape->connectFirst(m_shape1, m_firstHandleIndex);
        // If the shape selected is not the background
        if(tempShape != 0) {
            if(isInRoi()) {
                // If everything is good, we connect the line to the shape
                m_connectionShape->connectSecond(tempShape, getConnectionIndex(tempShape, m_mouse));
                m_isTied->second = true;
            } else {
                m_connectionShape->connectSecond(tempShape, 0);
            }
            
        } else {
        // If the cursor points the background
            if(isInRoi()) {
                // If everything is good, we connect the line to the shape
                m_connectionShape->connectSecond(tempShape, getConnectionIndex(tempShape, m_mouse));
                m_isTied->second = true;
            } else {
                m_connectionShape->moveHandle(m_connectionShape->handleCount(), event->point);
            }
        }
        // Will find the nearest point and update the connection shape
        updateConnections();
        
        // Apply the connection shape for now
        command();
        
        m_connectionShape = 0;
        m_lastShapeOn =0;
    }
}

void ConnectionTool::mouseMoveEvent(KoPointerEvent *event)
{
    // Record the last shape
    if(m_shapeOn != 0){
        m_lastShapeOn = m_shapeOn;
    }
    // Record the mouse position
    m_mouse = event->point;
    // Look at the new shape under the mouse
    m_shapeOn = canvas()->shapeManager()->shapeAt(event->point);

    KoConnectionShape * tempShape = dynamic_cast<KoConnectionShape*>(m_shapeOn);
    if(!tempShape) {
        if(m_connectionShape != 0) {
            if(isInRoi()) {
                // Make the connection to the specific point
                m_connectionShape->connectSecond(m_lastShapeOn, getConnectionIndex(m_lastShapeOn, m_mouse));
                m_connectionShape->updateConnections();
            } else if(m_shapeOn != 0) {
                // Make the connection to the first handle of the shape
                m_connectionShape->connectSecond(m_shapeOn, 0);
                updateConnections();
            } else {
                // Unmake the connection (detach it)
                m_connectionShape->connectSecond(0, 0);
                m_connectionShape->moveHandle(1, m_mouse);

                updateConnections();
            }
        }
    }
    // If we are really active we can follow the mouse with the line
    if(m_activeHandle != -1 && m_lastConnectionShapeOn != 0) {
        // We have to know what handle is actually moving
        if(m_activeHandle == 0){
            m_lastConnectionShapeOn->connectFirst(0, 0);
        }else if(m_activeHandle == 1){
            m_lastConnectionShapeOn->connectSecond(0, 0);
        }
        // We try to connect as usual, even if we are following the line
        if(!tempShape && isInRoi()) {
            if(m_lastShapeOn != 0){
                // We have to know what handle is actually moving
                // Connection with the specific handle
                if(m_activeHandle == 0){
                    m_lastConnectionShapeOn->connectFirst(m_lastShapeOn, getConnectionIndex(m_lastShapeOn, m_mouse));
                }else if(m_activeHandle == 1){
                    m_lastConnectionShapeOn->connectSecond(m_lastShapeOn, getConnectionIndex(m_lastShapeOn, m_mouse));
                }
            }
        }else if(m_shapeOn != 0 ){
            // We have to know what handle is actually moving
            // Connection with the first handle of the shape
            if(m_activeHandle == 0){
                m_lastConnectionShapeOn->connectFirst(m_shapeOn, 0);
            }else if(m_activeHandle == 1){
                m_lastConnectionShapeOn->connectSecond(m_shapeOn, 0);
            }
        }else{
            m_lastConnectionShapeOn->moveHandle(m_activeHandle, m_mouse);
        }
        m_lastConnectionShapeOn->updateConnections();
    }
    canvas()->updateCanvas(QRectF(0, 0, canvas()->canvasWidget()->width(), canvas()->canvasWidget()->height()));
}

void ConnectionTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if(event->modifiers() & Qt::ControlModifier) {
        if(isInRoi()) {
            // delete a connection Point
            m_shapeOn->removeConnectionPoint(getConnectionIndex(m_lastShapeOn, m_mouse));
        }else{
            // add a connection Point
            m_shapeOn = canvas()->shapeManager()->shapeAt(event->point);
            QPointF point = m_shapeOn->documentToShape(event->point);

            m_shapeOn->addConnectionPoint(point);
        }
    }else{
        if(m_modifyConnection){
            deactivate();
        }
    }
}

void ConnectionTool::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape) {
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
    m_shape1 = 0;
    m_lastConnectionShapeOn = 0;
    m_lastShapeOn = 0;
    m_modifyConnection = false;
    if(m_connectionShape != 0) {
        QRectF rec(m_connectionShape->boundingRect());
        canvas()->shapeManager()->remove(m_connectionShape);
        repaint(rec);
        canvas()->updateCanvas(rec);
        m_connectionShape = 0;
    }
}

void ConnectionTool::updateConnections()
{
    if(m_connectionShape == 0){
        return;
    }
    // If two shapes are connected
    if (m_connectionShape->firstShape() && m_connectionShape->secondShape()) {
        KoShape *shape1 = m_connectionShape->firstShape();
        KoShape *shape2 = m_connectionShape->secondShape();
        if(!m_isTied->first){
            m_connectionShape->connectFirst(shape1 , getConnectionIndex(shape1, shape2->absolutePosition()));
        }
        if(!m_isTied->second){
            m_connectionShape->connectSecond(shape2 , getConnectionIndex(shape2, shape1->absolutePosition()));
        }
    // If only the first item of the connection is a shape
    } else if(m_connectionShape->firstShape()) {
        KoShape *shape = m_connectionShape->firstShape();
        QPointF point = m_connectionShape->handlePosition(m_connectionShape->handleCount()) + m_connectionShape->absolutePosition();
        if(!m_isTied->first) {
            m_connectionShape->connectFirst(shape , getConnectionIndex(shape, point));
        }
    // If only the second item of the connection is a shape
    } else if(m_connectionShape->secondShape()) {
        KoShape* shape = m_connectionShape->secondShape();
        QPointF point = m_connectionShape->handlePosition(0) + m_connectionShape->absolutePosition();
        if(!m_isTied->second)
            m_connectionShape->connectSecond(shape , getConnectionIndex(shape, point));
    }
    // The connection is now done, so update and put everything to 0
    m_connectionShape->updateConnections();
}

int ConnectionTool::getConnectionIndex(KoShape * shape, QPointF point)
{
    float minDistance = HUGE_VAL;
    int nearestPointIndex = -1, i;
    // Get all the points
    QList<QPointF> connectionPoints = shape->connectionPoints();
    int connectionPointsCount = connectionPoints.count();

    point = shape->documentToShape(point);
    // Find the nearest point and stock the index
    for(i = 0; i < connectionPointsCount; i++)
    {
        float distance = distanceSquare(connectionPoints[i], point);
        if(distance < minDistance) {
            minDistance = distance;
            nearestPointIndex =  i;
        }
    }
    // return the nearest point index
    return nearestPointIndex;
}

float ConnectionTool::distanceSquare(QPointF p1, QPointF p2)
{
    // Square of the distance
    float distx = (p2.x() - p1.x()) * (p2.x() - p1.x());
    float disty = (p2.y() - p1.y()) * (p2.y() - p1.y());
    float dist = distx + disty;
    return dist;
}

bool ConnectionTool::isInRoi()
{
    int grabSensitivity = canvas()->resourceManager()->grabSensitivity() * canvas()->resourceManager()->grabSensitivity();
    if(m_lastShapeOn == 0)
        return false;
    
    KoConnectionShape * tempInRoiShape = dynamic_cast<KoConnectionShape*>(m_lastShapeOn);
    if(tempInRoiShape && m_connectionShape == 0)
        return false;
    
    QPointF mouse = m_lastShapeOn->documentToShape(m_mouse);
    foreach(const QPointF& point, m_lastShapeOn->connectionPoints())
        if(distanceSquare(mouse, point) <= grabSensitivity)
            return true;

    return false;
}

void ConnectionTool::command()
{
    // Create the command which will make the connection
    QUndoCommand * cmd = 0;
    if(m_connectionShape != 0)
        cmd = canvas()->shapeController()->addShape(m_connectionShape);

    if (cmd) {
        canvas()->addCommand(cmd);
    } else {
        canvas()->updateCanvas(m_connectionShape->boundingRect());
        delete m_connectionShape;
    }
}
