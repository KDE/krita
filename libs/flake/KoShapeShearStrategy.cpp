/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
#include "KoCanvasResourceProvider.h"
#include "commands/KoShapeShearCommand.h"
#include "commands/KoShapeMoveCommand.h"
#include "commands/KoShapeTransformCommand.h"

#include <QPointF>

#include <math.h>
#include <kdebug.h>
#include <klocale.h>

KoShapeShearStrategy::KoShapeShearStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked, KoFlake::SelectionHandle direction )
: KoInteractionStrategy(tool, canvas)
, m_start(clicked)
{
    KoSelection * sel = canvas->shapeManager()->selection();
    QList<KoShape*> selectedShapes = sel->selectedShapes(KoFlake::StrippedSelection);
    foreach(KoShape *shape, selectedShapes) {
        if( ! isEditable( shape ) )
            continue;
        m_selectedShapes << shape;
    }

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
    m_initialSize = sel->size();
    m_solidPoint = QPointF( m_initialSize.width() / 2, m_initialSize.height() / 2);

    if(m_top)
        m_solidPoint += QPointF(0, m_initialSize.height() / 2);
    else if(m_bottom)
        m_solidPoint -= QPointF(0, m_initialSize.height() / 2);
    if(m_left)
        m_solidPoint += QPointF(m_initialSize.width() / 2, 0);
    else if(m_right)
        m_solidPoint -= QPointF(m_initialSize.width() / 2, 0);

    QPointF edge;
    double angle = 0.0;
    if( m_top )
    {
        edge = sel->absolutePosition( KoFlake::BottomLeftCorner ) - sel->absolutePosition( KoFlake::BottomRightCorner );
        angle = 180.0;
    }
    else if( m_bottom )
    {
        edge = sel->absolutePosition( KoFlake::TopRightCorner ) - sel->absolutePosition( KoFlake::TopLeftCorner );
        angle = 0.0;
    }
    else if( m_left )
    {
        edge = sel->absolutePosition( KoFlake::BottomLeftCorner ) - sel->absolutePosition( KoFlake::TopLeftCorner );
        angle = 90.0;
    }
    else if( m_right )
    {
        edge = sel->absolutePosition( KoFlake::TopRightCorner ) - sel->absolutePosition( KoFlake::BottomRightCorner );
        angle = 270.0;
    }
    double currentAngle = atan2( edge.y(), edge.x() ) / M_PI * 180;
    m_initialSelectionAngle = currentAngle - angle;

    kDebug(30006) << " PREsol.x=" << m_solidPoint.x() << " sol.y=" << m_solidPoint.y() <<endl;
    m_solidPoint = canvas->shapeManager()->selection()->transformationMatrix(0).map( m_solidPoint );

    // use crossproduct of top edge and left edge of selection bounding rect
    // to determine if the selection is mirrored
    QPointF top = sel->absolutePosition( KoFlake::TopRightCorner ) - sel->absolutePosition( KoFlake::TopLeftCorner );
    QPointF left = sel->absolutePosition( KoFlake::BottomLeftCorner ) - sel->absolutePosition( KoFlake::TopLeftCorner );
    m_isMirrored = (top.x()*left.y() - top.y()*left.x() ) < 0.0;
}

void KoShapeShearStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    QPointF shearVector = point - m_start;

    QMatrix m;
    m.rotate(-m_initialSelectionAngle);
    shearVector = m.map(shearVector);

    double shearX=0, shearY=0;

    if(m_top || m_left)
        shearVector = - shearVector;
    if(m_top || m_bottom)
        shearX = shearVector.x() / m_initialSize.height();
    if(m_left || m_right)
        shearY = shearVector.y() / m_initialSize.width();

    // if selection is mirrored invert the shear values
    if( m_isMirrored )
    {
        shearX *= -1.0;
        shearY *= -1.0;
    }

    QMatrix matrix;
    matrix.translate(m_solidPoint.x(), m_solidPoint.y());
    matrix.rotate(m_initialSelectionAngle);
    matrix.shear(shearX, shearY);
    matrix.rotate(-m_initialSelectionAngle);
    matrix.translate(-m_solidPoint.x(), -m_solidPoint.y());

    QMatrix applyMatrix = matrix * m_shearMatrix.inverted();

    foreach( KoShape *shape, m_selectedShapes )
    {
        shape->repaint();
        shape->applyTransformation( applyMatrix );
        shape->repaint();
    }
    m_canvas->shapeManager()->selection()->applyTransformation( applyMatrix );
    m_shearMatrix = matrix;
}

void KoShapeShearStrategy::paint( QPainter &painter, const KoViewConverter &converter) {
    SelectionDecorator decorator(KoFlake::NoHandle, true, false);
    decorator.setSelection(m_canvas->shapeManager()->selection());
    decorator.setHandleRadius( m_canvas->resourceProvider()->handleRadius() );
    decorator.paint(painter, converter);
}

QUndoCommand* KoShapeShearStrategy::createCommand() {
    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( m_selectedShapes, m_shearMatrix );
    cmd->setText( i18n("Shear") );
    cmd->undo();
    return cmd;
}
