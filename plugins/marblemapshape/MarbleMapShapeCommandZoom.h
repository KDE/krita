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

#ifndef GEO_SHAPE_ZOOM_COMMAND
#define GEO_SHAPE_ZOOM_COMMAND

#include <kundo2command.h>

class MarbleMapShape;

/// The undo / redo command for zooming the map
class MarbleMapShapeCommandZoom : public KUndo2Command
{
public:
    /**
    * Command to zoom a map.
    *
    * @param shape the shape where we want to zoom
    * @param absolute zoom to value or simply zoom in (>0) or out (<0)
    * @param value absolute zoom value or direction
    * @param parent the parent command used for macro commands
     */
    MarbleMapShapeCommandZoom(MarbleMapShape * shape, signed int value, KUndo2Command *parent = 0);

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    MarbleMapShape *m_shape;
    signed int m_old_value;
    signed int m_new_value;
};


#endif
