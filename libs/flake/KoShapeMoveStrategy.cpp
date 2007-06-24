/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoShapeMoveStrategy.h"

#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoShapeContainer.h"
#include "KoShapeContainerModel.h"
#include "KoSelection.h"
#include "KoPointerEvent.h"
#include "KoCanvasResourceProvider.h"
#include "KoInteractionTool.h"
#include "commands/KoShapeTransformCommand.h"

#include <kdebug.h>
#include <klocale.h>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

KoShapeMoveStrategy::KoShapeMoveStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked)
: KoInteractionStrategy(tool, canvas)
, m_initialTopLeft(99999, 99999)
, m_start(clicked)
{
    QList<KoShape*> selectedShapes = canvas->shapeManager()->selection()->selectedShapes(KoFlake::TopLevelSelection);
    QRectF boundingRect;
    foreach(KoShape *shape, selectedShapes) {
        if( ! isEditable( shape ) )
            continue;
        m_selectedShapes << shape;
        boundingRect = boundingRect.unite( shape->boundingRect() );
    }
    m_initialTopLeft = boundingRect.topLeft();
}

void KoShapeMoveStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers) {
    if(m_selectedShapes.isEmpty()) return;
    QPointF diff = point - m_start;
    if(m_canvas->snapToGrid() && (modifiers & Qt::ShiftModifier) == 0) {
        QPointF newPos = m_initialTopLeft + diff;
        applyGrid(newPos);
        diff = newPos - m_initialTopLeft;
    }
    if(modifiers & (Qt::AltModifier | Qt::ControlModifier)) {
        // keep x or y position unchanged
        if(qAbs(diff.x()) < qAbs(diff.y()))
            diff.setX(0);
        else
            diff.setY(0);
    }

    QMatrix matrix;
    matrix.translate( diff.x(), diff.y() );
    QMatrix applyMatrix = matrix * m_translationMatrix.inverted();

    foreach( KoShape * shape, m_selectedShapes ) {
        shape->repaint();
        shape->applyTransformation( applyMatrix );
        shape->repaint();
    }
    m_canvas->shapeManager()->selection()->applyTransformation( applyMatrix );
    m_translationMatrix = matrix;
}

QUndoCommand* KoShapeMoveStrategy::createCommand() {
    if( m_translationMatrix.isIdentity() )
        return 0;
    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( m_selectedShapes, m_translationMatrix );
    cmd->setText( i18n( "Move shapes" ) );
    cmd->undo();
    return cmd;
}

void KoShapeMoveStrategy::paint( QPainter &painter, const KoViewConverter &converter) {
    SelectionDecorator decorator (KoFlake::NoHandle, false, false);
    decorator.setSelection(m_canvas->shapeManager()->selection());
    decorator.setHandleRadius( m_canvas->resourceProvider()->handleRadius() );
    decorator.paint(painter, converter);
}
