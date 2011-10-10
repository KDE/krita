/* TPart of Calligra Suite - Map Shape
   Copyright 2008 Simon Schmeisser <mail_to_wrt@gmx.de>
   Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>

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
   Boston, MA 02110-1301, USA.
*/

#include "MapShapeCommandChangeProjection.h"
#include "MapShape.h"

#include <MarbleWidget.h>

MapShapeCommandChangeProjection::MapShapeCommandChangeProjection(MapShape * shape, Marble::Projection projection, KUndo2Command *parent)
: KUndo2Command(parent)
{
    m_shape = shape;
    m_new_projection = projection;

    if (m_shape)
        m_old_projection = m_shape->marbleWidget()->projection();

    redo();
}

void MapShapeCommandChangeProjection::redo()
{
    if (m_shape) {
        m_shape->marbleWidget()->setProjection(m_new_projection);
        m_shape->update();
    }
}

void MapShapeCommandChangeProjection::undo()
{
    if (m_shape) {
        m_shape->marbleWidget()->setProjection(m_old_projection);
        m_shape->update();
    }
}
