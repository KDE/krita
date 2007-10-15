/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeRotateStrategy.h"
#include "KoInteractionTool.h"
#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "KoShapeManager.h"
#include "KoCanvasResourceProvider.h"
#include "commands/KoShapeTransformCommand.h"

#include <QPointF>
#include <math.h>
#include <klocale.h>

KoShapeRotateStrategy::KoShapeRotateStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked)
: KoInteractionStrategy(tool, canvas)
, m_initialBoundingRect()
, m_start(clicked)
{
    QList<KoShape*> selectedShapes = canvas->shapeManager()->selection()->selectedShapes(KoFlake::StrippedSelection);
    foreach(KoShape *shape, selectedShapes) {
        if( ! isEditable( shape ) )
            continue;
        m_selectedShapes << shape;
        if( m_selectedShapes.count() == 1 )
            m_initialBoundingRect = shape->boundingRect();
        else
            m_initialBoundingRect = m_initialBoundingRect.united( shape->boundingRect() );
        m_oldTransforms << shape->transformation();
    }
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

    QMatrix applyMatrix = matrix * m_rotationMatrix.inverted();
    m_rotationMatrix = matrix;
    foreach( KoShape * shape, m_selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( applyMatrix );
        shape->update();
    }
    m_canvas->shapeManager()->selection()->applyAbsoluteTransformation( applyMatrix );
}

void KoShapeRotateStrategy::paint( QPainter &painter, const KoViewConverter &converter) {
    SelectionDecorator decorator(KoFlake::NoHandle, true, false);
    decorator.setSelection(m_canvas->shapeManager()->selection());
    decorator.setHandleRadius( m_canvas->resourceProvider()->handleRadius() );
    decorator.paint(painter, converter);

    // paint the rotation center
    painter.setPen( QPen(Qt::red));
    painter.setBrush( QBrush(Qt::red));
    painter.setRenderHint( QPainter::Antialiasing, true );
    QRectF circle( 0, 0, 5, 5 );
    circle.moveCenter( converter.documentToView( m_initialBoundingRect.center() ) );
    painter.drawEllipse( circle );
}

QUndoCommand* KoShapeRotateStrategy::createCommand() {
    QList<QMatrix> newTransforms;
    foreach( KoShape* shape, m_selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( m_selectedShapes, m_oldTransforms, newTransforms );
    cmd->setText( i18n("Rotate") );
    return cmd;
}
