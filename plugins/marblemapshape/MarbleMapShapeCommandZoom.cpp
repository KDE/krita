/* Part of Calligra Suite - Marble Map Shape
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

#include "MarbleMapShapeCommandZoom.h"
#include "MarbleMapShape.h"

#include <MarbleWidget.h>


MarbleMapShapeCommandZoom::MarbleMapShapeCommandZoom(MarbleMapShape * shape, signed int value, KUndo2Command *parent)
: KUndo2Command(parent)
{
    m_shape = shape;
    m_new_value = value;

    if (m_shape)
        m_old_value = m_shape->marbleWidget()->zoom();

    redo();
}

void MarbleMapShapeCommandZoom::redo()
{
    if (m_shape) {
        m_shape->marbleWidget()->zoomView(m_new_value);
        m_shape->update();
    }
}

void MarbleMapShapeCommandZoom::undo()
{
    if (m_shape) {
        m_shape->marbleWidget()->zoomView(m_old_value);
        m_shape->update();
    }
}
