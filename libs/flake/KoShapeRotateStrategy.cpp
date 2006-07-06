/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeRotateStrategy.h"
#include "KoInteractionTool.h"
#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "KoShapeManager.h"
#include "KoCommand.h"

#include <QPointF>

#include <math.h>

KoShapeRotateStrategy::KoShapeRotateStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked)
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
        m_startAbsolutePositions << shape->absolutePosition();
        m_initialAngles << shape->rotation();
        m_initialBoundingRect = m_initialBoundingRect.unite( shape->boundingRect() );
    }
    m_initialSelectionAngle = canvas->shapeManager()->selection()->rotation();
}

void KoShapeRotateStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers) {
    QPointF center = m_initialBoundingRect.center();
    double angle = atan2( point.y() - center.y(), point.x() - center.x() ) -
        atan2( m_start.y() - center.y(), m_start.x() - center.x() );
    angle = angle / M_PI * 180;  // convert to degrees.
    if(modifiers & (Qt::AltModifier | Qt::ControlModifier)) {
        // limit to 45 degree angles
        double modula = qAbs(angle);
        while(modula > 45.0)
            modula -= 45.0;
        if(modula > 22.5)
            modula -= 45.0;
        angle += (angle>0?-1:1)*modula;
    }

    QMatrix matrix;
    matrix.translate(center.x(), center.y());
    matrix.rotate(angle);
    matrix.translate(-center.x(), -center.y());

    int counter=0;
    foreach(KoShape *shape, m_selectedShapes) {
        shape->repaint();
        shape->setAbsolutePosition(matrix.map(m_startAbsolutePositions[counter]));
        shape->rotate(m_initialAngles[counter] + angle);
        shape->repaint();
        counter++;
    }
    m_canvas->shapeManager()->selection()->rotate(m_initialSelectionAngle + angle);

}

void KoShapeRotateStrategy::paint( QPainter &painter, KoViewConverter &converter) {
    SelectionDecorator decorator(KoFlake::NoHandle, true, false);
    decorator.setSelection(m_canvas->shapeManager()->selection());
    decorator.paint(painter, converter);
}

KCommand* KoShapeRotateStrategy::createCommand() {
    KMacroCommand *cmd = new KMacroCommand("Rotate");
    QList<QPointF> newPositions;
    QList<double> newAngles;
    foreach(KoShape *shape, m_selectedShapes) {
        newPositions << shape->position();
        newAngles << shape->rotation();
    }
    cmd->addCommand(new KoShapeMoveCommand(m_selectedShapes, m_startPositions, newPositions));
    cmd->addCommand(new KoShapeRotateCommand(m_selectedShapes, m_initialAngles, newAngles));
    return cmd;
}
