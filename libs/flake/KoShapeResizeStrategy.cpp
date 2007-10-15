/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KoShapeResizeStrategy.h"
#include "KoShapeManager.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "KoCanvasResourceProvider.h"
#include "commands/KoShapeSizeCommand.h"
#include "commands/KoShapeTransformCommand.h"

#include <klocale.h>

KoShapeResizeStrategy::KoShapeResizeStrategy( KoTool *tool, KoCanvasBase *canvas,
        const QPointF &clicked, KoFlake::SelectionHandle direction )
: KoInteractionStrategy(tool, canvas)
{
    Q_ASSERT( canvas->shapeManager()->selection()->count() > 0);
    QList<KoShape*> selectedShapes = canvas->shapeManager()->selection()->selectedShapes(KoFlake::StrippedSelection);
    foreach(KoShape *shape, selectedShapes) {
        if( ! isEditable( shape ) )
            continue;
        m_selectedShapes << shape;
        m_startPositions << shape->position();
        m_oldTransforms << shape->transformation();
        m_transformations << QMatrix();
        m_startSizes << shape->size();
    }
    m_start = clicked;

    KoShape *shp = 0;
    if(canvas->shapeManager()->selection()->count()>1)
       shp = canvas->shapeManager()->selection();
    if(canvas->shapeManager()->selection()->count()==1)
        shp = canvas->shapeManager()->selection()->firstSelectedShape();

    if( shp )
    {
        m_windMatrix = shp->absoluteTransformation(0);
        m_unwindMatrix = m_windMatrix.inverted();
        m_initialSize = shp->size();
        m_initialPosition = m_windMatrix.map(QPointF());
    }

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
             Q_ASSERT(0); // illegal 'corner'
    }
}

void KoShapeResizeStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers) {
    QPointF newPos = point;
    if(m_canvas->snapToGrid() && (modifiers & Qt::ShiftModifier) == 0)
        applyGrid(newPos);

    bool keepAspect = modifiers & Qt::AltModifier;
    foreach(KoShape *shape, m_selectedShapes)
        keepAspect = keepAspect || shape->keepAspectRatio();

    double startWidth = m_initialSize.width();
    double startHeight = m_initialSize.height();

    QPointF distance = m_unwindMatrix.map(newPos) - m_unwindMatrix.map( m_start );

    double zoomX=1, zoomY=1;
    if(m_left)
        zoomX = (startWidth - distance.x()) / startWidth;
    else if(m_right)
        zoomX = (startWidth + distance.x()) / startWidth;
    if(m_top)
        zoomY = (startHeight - distance.y()) / startHeight;
    else if(m_bottom)
        zoomY = (startHeight + distance.y()) / startHeight;
    if(keepAspect) {
        const bool cornerUsed = ((m_bottom?1:0) + (m_top?1:0) + (m_left?1:0) + (m_right?1:0)) == 2;
        if(cornerUsed && startWidth < startHeight || m_left || m_right)
            zoomY = zoomX;
        else
            zoomX = zoomY;
    }

    bool scaleFromCenter = modifiers & Qt::ControlModifier;
    QPointF move;
    QMatrix matrix;

    if(scaleFromCenter)
        move = QPointF(startWidth / 2.0, startHeight / 2.0);
    else
        move = QPointF(m_left?startWidth:0, m_top?startHeight:0);

    matrix.translate(move.x(), move.y()); // translate to 
    matrix.scale(zoomX, zoomY);
    matrix.translate(-move.x(), -move.y()); // and back

    // that is the transformation we want to apply to the shapes
    matrix = m_unwindMatrix * matrix * m_windMatrix;

    // the resizing transformation without the mirroring part
    QMatrix resizeMatrix;
    resizeMatrix.translate(move.x(), move.y()); // translate to 
    resizeMatrix.scale( qAbs(zoomX), qAbs(zoomY) );
    resizeMatrix.translate(-move.x(), -move.y()); // and back

    // the mirroring part of the resizing transformation
    QMatrix mirrorMatrix;
    mirrorMatrix.translate(move.x(), move.y()); // translate to 
    mirrorMatrix.scale( zoomX < 0 ? -1 : 1, zoomY < 0 ? -1 : 1 );
    mirrorMatrix.translate(-move.x(), -move.y()); // and back

    int i = 0;
    foreach(KoShape *shape, m_selectedShapes)
    {
        shape->update();

        // this uses resize for the zooming part
        shape->applyAbsoluteTransformation( m_unwindMatrix );

        /*
         normally we would just apply the resizeMatrix now and be done with it, but
         we want to resize instead of scale, so we have to separate the scaling part
         of that transformation which can then be used to resize
        */

        // undo the last resize transformation
        shape->applyAbsoluteTransformation( m_transformations[i].inverted() );

        // save the shapes transformation matrix
        QMatrix shapeMatrix = shape->absoluteTransformation(0);

        // calculate the matrix we would apply to the local shape matrix
        // that tells us the effective scale values we have to use for the resizing
        QMatrix localMatrix = shapeMatrix * resizeMatrix * shapeMatrix.inverted();
        // save the effective scale values
        double scaleX = localMatrix.m11();
        double scaleY = localMatrix.m22();

        // calculate the scale matrix which is equivalent to our resizing above
        QMatrix scaleMatrix = (QMatrix().scale( scaleX, scaleY ));
        scaleMatrix =  shapeMatrix.inverted() * scaleMatrix * shapeMatrix;

        // calculate the new size of the shape, using the effective scale values
        QSizeF size( scaleX * m_startSizes[i].width(), scaleY * m_startSizes[i].height() );

        // apply the transformation
        shape->setSize( size );
        // apply the rest of the transformation without the resizing part
        shape->applyAbsoluteTransformation( scaleMatrix.inverted() * resizeMatrix );
        shape->applyAbsoluteTransformation( mirrorMatrix );

        // and remember the applied transformation later for later undoing
        m_transformations[i] = shapeMatrix.inverted() * shape->absoluteTransformation(0);

        shape->applyAbsoluteTransformation( m_windMatrix );

        shape->update();
        i++;
    }
    m_canvas->shapeManager()->selection()->applyAbsoluteTransformation( matrix * m_scaleMatrix.inverted() );
    m_scaleMatrix = matrix;
}

QUndoCommand* KoShapeResizeStrategy::createCommand() {
    QList<QSizeF> newSizes;
    QList<QMatrix> transformations;
    const int shapeCount = m_selectedShapes.count();
    for( int i = 0; i < shapeCount; ++i )
    {
        newSizes << m_selectedShapes[i]->size();
        transformations << m_selectedShapes[i]->transformation();
    }
    QUndoCommand * cmd = new QUndoCommand(i18n("Resize"));
    new KoShapeSizeCommand(m_selectedShapes, m_startSizes, newSizes, cmd );
    new KoShapeTransformCommand( m_selectedShapes, m_oldTransforms, transformations, cmd );
    return cmd;
}

void KoShapeResizeStrategy::paint( QPainter &painter, const KoViewConverter &converter) {
    SelectionDecorator decorator (KoFlake::NoHandle, false, false);
    decorator.setSelection(m_canvas->shapeManager()->selection());
    decorator.setHandleRadius( m_canvas->resourceProvider()->handleRadius() );
    decorator.paint(painter, converter);
}
