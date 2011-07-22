/* This file is part of the KDE project

   Copyright (c) 2010 Cyril Oblikov <munknex@gmail.com>

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

#include "SelectionDecorator.h"

#include <KoShape.h>
#include <KoSelection.h>

SelectionDecorator::SelectionDecorator()
{
}

SelectionDecorator::~SelectionDecorator()
{
}

void SelectionDecorator::setSelection(KoSelection *selection) {
    m_selection = selection;
}

void SelectionDecorator::paint(QPainter &painter, const KoViewConverter &converter) {
    painter.save();

    // save the original painter transformation
    QTransform painterMatrix = painter.worldTransform();

    painter.setPen( Qt::green );
    foreach(KoShape *shape, m_selection->selectedShapes(KoFlake::StrippedSelection)) {
        // apply the shape transformation on top of the old painter transformation
        painter.setWorldTransform( shape->absoluteTransformation(&converter) * painterMatrix );
        // apply the zoom factor
        KoShape::applyConversion( painter, converter );
        // draw the shape bounding rect
        painter.drawRect( QRectF( QPointF(), shape->size() ) );

    }

    if(m_selection->count()>1) {
        // more than one shape selected, so we need to draw the selection bounding rect
        painter.setPen( Qt::blue );
        // apply the selection transformation on top of the old painter transformation
        painter.setWorldTransform(m_selection->absoluteTransformation(&converter) * painterMatrix);
        // apply the zoom factor
        KoShape::applyConversion( painter, converter );
        // draw the selection bounding rect
        painter.drawRect( QRectF( QPointF(), m_selection->size() ) );
    } else if( m_selection->firstSelectedShape() ) {
        // only one shape selected, so we compose the correct painter matrix
        painter.setWorldTransform(m_selection->firstSelectedShape()->absoluteTransformation(&converter) * painterMatrix);
        KoShape::applyConversion( painter, converter );
    }
    painter.restore();
}

