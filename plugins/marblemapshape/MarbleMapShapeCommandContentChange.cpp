/*  Part of Calligra Suite - Marble Map Shape
    Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "MarbleMapShapeCommandContentChange.h"

#include "MarbleMapShape.h"
#include <MarbleWidget.h>
#include <QPointF>


MarbleMapShapeCommandContentChange::MarbleMapShapeCommandContentChange(
    MarbleMapShape* shape,
    QPointF value,
    KUndo2Command* parent)
    : KUndo2Command(parent)
{
    m_shape = shape;
    m_new_value = value;
    
    if (m_shape){
        m_old_value.setX(m_shape->marbleWidget()->centerLongitude());
        m_old_value.setY(m_shape->marbleWidget()->centerLatitude());
    }
    redo();
}


void MarbleMapShapeCommandContentChange::undo()
{
    if (m_shape) {
        m_shape->marbleWidget()->centerOn(m_new_value.x(), m_new_value.y());
        m_shape->update();
    }
}

void MarbleMapShapeCommandContentChange::redo()
{
    if (m_shape) {
        m_shape->marbleWidget()->centerOn(m_old_value.x(), m_old_value.y());
        m_shape->update();
    }
}