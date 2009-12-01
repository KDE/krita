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

#include "KoConnectionTool.h"

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoShapeFactory.h>
#include <KoShape.h>
#include <KoShapeController.h>
#include <KoShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoSelection.h>
#include <KoLineBorder.h>
#include <KoCanvasResourceProvider.h>

#include <QUndoCommand>
#include <QPointF>
#include <QKeyEvent>

KoConnectionTool::KoConnectionTool(KoCanvasBase * canvas)
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

KoConnectionTool::~KoConnectionTool()
{
    delete m_isTied;
}

void KoConnectionTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    // get the correctly sized rect for painting handles
    QRectF handleRect = handlePaintRect(QPointF());

    painter.setRenderHint(QPainter::Antialiasing, true);
    // Green rects
    if(m_shapeOn !=  0) {
        // save the painter to restore it later
        painter.save();
        // Apply the conversion make by the matrix transformation
        QMatrix transform = m_shapeOn->absoluteTransformation(0);
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
        QMatrix transform = m_lastShapeOn->absoluteTransformation(0);
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
        int radius = m_canvas->resourceProvider()->handleRadius();
        // Apply the conversion make by the matrix transformation
        painter.setMatrix(tempShape->absoluteTransformation(&converter) * painter.matrix());
        // ... handle unselected
        tempShape->paintHandles(painter, converter, radius);

        painter.restore();

        int grabSensitivity = m_canvas->resourceProvider()->grabSensitivity();
        QRectF rec(m_mouse.x()-grabSensitivity/2, m_mouse.y()-grabSensitivity/2, grabSensitivity, grabSensitivity);
        int handleId = tempShape->handleIdAt(tempShape->documentToShape(rec));

        if(handleId != -1) {
            painter.save();
            // ... handle selected
            painter.setPen(Qt::blue);
            painter.setBrush(Qt::red);
            // Apply the conversion make by the matrix transformation
            painter.setMatrix(tempShape->absoluteTransformation(&converter) * painter.matrix());
            tempShape->paintHandle(painter, converter, handleId, radius);

            painter.restore();
        }
    }
}

void KoConnectionTool::mousePressEvent(KoPointerEvent *event)
{
    // If we are pressing down the control key, it should add handles, not connectionShape
    if(event->modifiers() & Qt::ControlModifier)
        return;
    // We here try to recover the the last shape in case of the handle point is out of the shape
    KoShape * tempShape = 0;
    if(isInRoi())
        tempShape = m_lastShapeOn;
    else
        tempShape = m_canvas->shapeManager()->shapeAt(event->point);
    
    // We take care if the shape under the mouse is not another connection shape
    KoConnectionShape * tempConnectionShape = dynamic_cast<KoConnectionShape*>(m_shapeOn);
    if(tempConnectionShape && m_connectionShape == 0){
        // grabSensitivity is defined by the user
        int grabSensitivity = m_canvas->resourceProvider()->grabSensitivity();
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
        KoShapeFactory *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
        KoShape *shape = factory->createDefaultShapeAndInit(m_canvas->shapeController()->dataCenterMap());
        if((m_connectionShape = dynamic_cast<KoConnectionShape*>(shape))){
            KoConnectionShape * connectionShapeTest = dynamic_cast<KoConnectionShape*>(tempShape);
            if(isInRoi()) {
                m_shape1 = tempShape;
                m_firstHandleIndex = getConnectionIndex(tempShape, m_mouse);
                
                m_connectionShape->setConnection1(m_shape1, m_firstHandleIndex);
                m_isTied->first = true;
            // If the shape selected is not the background
            // We take care if the working tempShape is not another connection shape
            }else if(tempShape != 0 && !connectionShapeTest) {
                m_shape1 = tempShape;
                m_firstHandleIndex = 0;
                m_connectionShape->setConnection1(m_shape1, m_firstHandleIndex);
                m_isTied->first = false;
            } else {
                m_shape1 = 0;
                m_firstHandleIndex = 0;
                m_connectionShape->moveHandle(0, event->point);
            }
            
            m_connectionShape->moveHandle(1, event->point);
            // The connection is now done, so update for apply
            m_connectionShape->updateConnections();
            m_canvas->shapeManager()->add(m_connectionShape);
        }
    } else {
    // Second click
    
        if(m_shape1 != 0)
            m_connectionShape->setConnection1(m_shape1, m_firstHandleIndex);
        // If the shape selected is not the background
        if(tempShape != 0) {
            if(isInRoi()) {
                // If everything is good, we connect the line to the shape
                m_connectionShape->setConnection2(tempShape, getConnectionIndex(tempShape, m_mouse));
                m_isTied->second = true;
            } else {
                m_connectionShape->setConnection2(tempShape, 0);
            }
            
        } else {
        // If the cursor points the background
            if(isInRoi()) {
                // If everything is good, we connect the line to the shape
                m_connectionShape->setConnection2(tempShape, getConnectionIndex(tempShape, m_mouse));
                m_isTied->second = true;
            } else {
                m_connectionShape->moveHandle(m_connectionShape->getHandleCount(), event->point);
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

void KoConnectionTool::mouseMoveEvent(KoPointerEvent *event)
{
    // Record the last shape
    if(m_shapeOn != 0){
        m_lastShapeOn = m_shapeOn;
    }
    // Record the mouse position
    m_mouse = event->point;
    // Look at the new shape under the mouse
    m_shapeOn = m_canvas->shapeManager()->shapeAt(event->point);

    KoConnectionShape * tempShape = dynamic_cast<KoConnectionShape*>(m_shapeOn);
    if(!tempShape) {
        if(m_connectionShape != 0) {
            if(isInRoi()) {
                // Make the connection to the specific point
                m_connectionShape->setConnection2(m_lastShapeOn, getConnectionIndex(m_lastShapeOn, m_mouse));
                m_connectionShape->updateConnections();
            } else if(m_shapeOn != 0) {
                // Make the connection to the first handle of the shape
                m_connectionShape->setConnection2(m_shapeOn, 0);
                updateConnections();
            } else {
                // Unmake the connection (detach it)
                m_connectionShape->setConnection2(0, 0);
                m_connectionShape->moveHandle(1, m_mouse);

                updateConnections();
            }
        }
    }
    // If we are really active we can follow the mouse with the line
    if(m_activeHandle != -1 && m_lastConnectionShapeOn != 0) {
        // We have to know what handle is actually moving
        if(m_activeHandle == 0){
            m_lastConnectionShapeOn->setConnection1(0, 0);
        }else if(m_activeHandle == 1){
            m_lastConnectionShapeOn->setConnection2(0, 0);
        }
        // We try to connect as usual, even if we are following the line
        if(!tempShape && isInRoi()) {
            if(m_lastShapeOn != 0){
                // We have to know what handle is actually moving
                // Connection with the specific handle
                if(m_activeHandle == 0){
                    m_lastConnectionShapeOn->setConnection1(m_lastShapeOn, getConnectionIndex(m_lastShapeOn, m_mouse));
                }else if(m_activeHandle == 1){
                    m_lastConnectionShapeOn->setConnection2(m_lastShapeOn, getConnectionIndex(m_lastShapeOn, m_mouse));
                }
            }
        }else if(m_shapeOn != 0 ){
            // We have to know what handle is actually moving
            // Connection with the first handle of the shape
            if(m_activeHandle == 0){
                m_lastConnectionShapeOn->setConnection1(m_shapeOn, 0);
            }else if(m_activeHandle == 1){
                m_lastConnectionShapeOn->setConnection2(m_shapeOn, 0);
            }
        }else{
            m_lastConnectionShapeOn->moveHandle(m_activeHandle, m_mouse);
        }
        m_lastConnectionShapeOn->updateConnections();
    }
    m_canvas->updateCanvas(QRectF(0, 0, m_canvas->canvasWidget()->width(), m_canvas->canvasWidget()->height()));
}

void KoConnectionTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if(event->modifiers() & Qt::ControlModifier) {
        if(isInRoi()) {
            // delete a connection Point
            m_shapeOn->removeConnectionPoint(getConnectionIndex(m_lastShapeOn, m_mouse));
        }else{
            // add a connection Point
            m_shapeOn = m_canvas->shapeManager()->shapeAt(event->point);
            QPointF point = m_shapeOn->documentToShape(event->point);

            m_shapeOn->addConnectionPoint(point);
        }
    }else{
        if(m_modifyConnection){
            deactivate();
        }
    }
}

void KoConnectionTool::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape) {
        deactivate();
    }
}

void KoConnectionTool::activate(bool temporary)
{
    Q_UNUSED(temporary);
    m_canvas->canvasWidget()->setCursor(Qt::PointingHandCursor);
}

void KoConnectionTool::deactivate()
{
    // Put everything to 0 to be able to begin a new shape properly
    m_shape1 = 0;
    m_lastConnectionShapeOn = 0;
    m_lastShapeOn = 0;
    m_modifyConnection = false;
    if(m_connectionShape != 0) {
        QRectF rec(m_connectionShape->boundingRect());
        m_canvas->shapeManager()->remove(m_connectionShape);
        repaint(rec);
        m_canvas->updateCanvas(rec);
        m_connectionShape = 0;
    }
}

void KoConnectionTool::updateConnections()
{
    if(m_connectionShape == 0){
        return;
    }
    KoConnection connection1 = m_connectionShape->connection1();
    KoConnection connection2 = m_connectionShape->connection2();

    // If two shapes are connected
    if(connection1.first != 0 && connection2.first != 0) {
        KoShape* shape1 = connection1.first;
        KoShape* shape2 = connection2.first;
        if(!m_isTied->first){
            m_connectionShape->setConnection1(shape1 , getConnectionIndex(shape1, shape2->absolutePosition()));
        }
        if(!m_isTied->second){
            m_connectionShape->setConnection2(shape2 , getConnectionIndex(shape2, shape1->absolutePosition()));
        }
    // If only the first item of the connection is a shape
    } else if(connection1.first != 0) {
        KoShape* shape = connection1.first;
        QPointF point = m_connectionShape->handlePosition(m_connectionShape->getHandleCount()) + m_connectionShape->absolutePosition();
        if(!m_isTied->first) {
            m_connectionShape->setConnection1(shape , getConnectionIndex(shape, point));
        }
    // If only the second item of the connection is a shape
    } else if(connection2.first != 0) {
        KoShape* shape = connection2.first;
        QPointF point = m_connectionShape->handlePosition(0) + m_connectionShape->absolutePosition();
        if(!m_isTied->second)
            m_connectionShape->setConnection2(shape , getConnectionIndex(shape, point));
    }
    // The connection is now done, so update and put everything to 0
    m_connectionShape->updateConnections();
}

int KoConnectionTool::getConnectionIndex(KoShape * shape, QPointF point)
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

float KoConnectionTool::distanceSquare(QPointF p1, QPointF p2)
{
    // Square of the distance
    float distx = (p2.x() - p1.x()) * (p2.x() - p1.x());
    float disty = (p2.y() - p1.y()) * (p2.y() - p1.y());
    float dist = distx + disty;
    return dist;
}

bool KoConnectionTool::isInRoi()
{
    int grabSensitivity = m_canvas->resourceProvider()->grabSensitivity() * m_canvas->resourceProvider()->grabSensitivity();
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

void KoConnectionTool::command()
{
    // Create the command which will make the connection
    QUndoCommand * cmd = 0;
    if(m_connectionShape != 0)
        cmd = m_canvas->shapeController()->addShape(m_connectionShape);

    if (cmd) {
        m_canvas->addCommand(cmd);
    } else {
        m_canvas->updateCanvas(m_connectionShape->boundingRect());
        delete m_connectionShape;
    }
}
