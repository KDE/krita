/*  Part of Calligra Suite - Map Shape
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


#ifndef MAPSHAPECOMMANDCONTENTCHANGE_H
#define MAPSHAPECOMMANDCONTENTCHANGE_H

#include <kundo2command.h>

class MapShape;
class QPointF;

class MapShapeCommandContentChange : public KUndo2Command
{

public:
    explicit MapShapeCommandContentChange(MapShape * shape, QPointF value, KUndo2Command* parent = 0);
    virtual void undo();
    virtual void redo();
private:
    MapShape *m_shape;
    QPointF m_old_value;
    QPointF m_new_value;
};

#endif // MAPSHAPECOMMANDCONTENTCHANGE_H
