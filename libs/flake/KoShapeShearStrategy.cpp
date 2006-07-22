/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 casper Boemann <cbr@boemann.dk>
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

#include "KoShapeShearStrategy.h"
#include "KoInteractionTool.h"
#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "KoShapeManager.h"
#include "KoCommand.h"

#include <QPointF>

#include <math.h>
#include <kdebug.h>

KoShapeShearStrategy::KoShapeShearStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked, KoFlake::SelectionHandle direction )
: KoInteractionStrategy(tool, canvas)
, m_initialBoundingRect()
, m_start(clicked)
{
    KoSelectionSet selectedShapes = canvas->shapeManager()->selection()->selectedShapes(KoFlake::StrippedSelection);
    foreach(KoShape *shape, selectedShapes) {
        if(shape->isLocked())
            continue;
        m_selectedShapes << shape;
        m_startPositions << shape->position();
        m_startMatrices << shape->transformationMatrix(0);
        QMatrix m;
        m.rotate(shape->rotation());
        m_startMatrices << m;
        m_startShearXs << shape->shearX();
        m_startShearYs << shape->shearY();
        m_initialBoundingRect = m_initialBoundingRect.unite( shape->boundingRect() );
    }
    m_initialSelectionAngle = canvas->shapeManager()->selection()->rotation();

    // Eventhoug we aren't currently activated by the corner handles we might as well code like it
    switch(direction) {
        case KoFlake::TopMiddleHandle:
            m_top = true; m_bottom = false; m_left = false; m_right = false; break;
        case KoFlake::TopRightHandle:
            m_top = true; m_bottom = false; m_left = false; m_right = true; break;
        case KoFlake::RightMiddleHandle:
            m_top = false; m_bottom = false; m_left = false; m_right = true; break;
        case KoFlake::BottomRightHandle:
            m_top = false; m_bottom = true; m_left = false; m_right = true; break;
        case KoFlake::BottomMiddleHandle:
            m_top = false; m_bottom = true; m_left = false; m_right = false; break;
        case KoFlake::BottomLeftHandle:
            m_top = false; m_bottom = true; m_left = true; m_right = false; break;
        case KoFlake::LeftMiddleHandle:
            m_top = false; m_bottom = false; m_left = true; m_right = false; break;
        case KoFlake::TopLeftHandle:
            m_top = true; m_bottom = false; m_left = true; m_right = false; break;
        default:
            ;// throw exception ?  TODO
    }
    m_initialSize = canvas->shapeManager()->selection()->size();
    m_solidPoint = QPointF( m_initialSize.width() / 2, m_initialSize.height() / 2);

    if(m_top)
        m_solidPoint += QPointF(0, m_initialSize.height() / 2);
    else if(m_bottom)
        m_solidPoint -= QPointF(0, m_initialSize.height() / 2);
    if(m_left)
        m_solidPoint += QPointF(m_initialSize.width() / 2, 0);
    else if(m_right)
        m_solidPoint -= QPointF(m_initialSize.width() / 2, 0);

kDebug() << " PREsol.x=" << m_solidPoint.x() << " sol.y=" << m_solidPoint.y() <<endl;
        QMatrix matrix = canvas->shapeManager()->selection()->transformationMatrix(0);
        m_solidPoint = matrix.map(m_solidPoint);
}

void KoShapeShearStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    QPointF shearVector = point - m_solidPoint;

    QMatrix matrix;
    matrix.rotate(-m_initialSelectionAngle);
    shearVector = matrix.map(shearVector);

    double shearX=0, shearY=0;

    if(m_top || m_left)
        shearVector = - shearVector;
    if(m_top || m_bottom)
        shearX = shearVector.x() / m_initialSize.height();
    if(m_left || m_right)
        shearY = shearVector.y() / m_initialSize.width();

    QMatrix applyMatrix;
    applyMatrix.translate(m_solidPoint.x(), m_solidPoint.y());
    applyMatrix.shear(shearX, shearY);
    applyMatrix.translate(-m_solidPoint.x(), -m_solidPoint.y());

kDebug() << "Begin retransform" <<endl;
    int counter=0;
    foreach(KoShape *shape, m_selectedShapes) {
        shape->repaint();
        QMatrix m = m_startMatrices[counter] * applyMatrix;
        shape->shear(m.m21(), m.m12());
kDebug() << " s.dx=" << m_startMatrices[counter].dx() << " s.dy=" << m_startMatrices[counter].dy() <<endl;
kDebug() << " m.dx=" << m.dx() << " m.dy=" << m.dy() <<endl;
QPointF p = applyMatrix.map(m_startPositions[counter]);
kDebug() << " px=" << p.x() << " py=" << p.y() <<endl;
        shape->setPosition(p);//QPointF(m.dx(), m.dy()));
        shape->repaint();
        counter++;
    }
    m_canvas->shapeManager()->selection()->shear(shearX, shearY);
    // The selection updates it's own position and size
}

void KoShapeShearStrategy::paint( QPainter &painter, KoViewConverter &converter) {
    SelectionDecorator decorator(KoFlake::NoHandle, true, false);
    decorator.setSelection(m_canvas->shapeManager()->selection());
    decorator.paint(painter, converter);
}

KCommand* KoShapeShearStrategy::createCommand() {
    KMacroCommand *cmd = new KMacroCommand("Shear");
    QList<QPointF> newPositions;
    QList<double> newShearX;
    QList<double> newShearY;
    foreach(KoShape *shape, m_selectedShapes) {
        newPositions << shape->position();
        newShearX << shape->shearX();
        newShearY << shape->shearY();
    }
    cmd->addCommand(new KoShapeMoveCommand(m_selectedShapes, m_startPositions, newPositions));
    cmd->addCommand(new KoShapeShearCommand(m_selectedShapes, m_startShearXs, m_startShearYs, newShearX, newShearY));
    return cmd;
}
