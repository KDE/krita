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

#ifndef GEO_SHAPE_CHANGE_PROJECTION_COMMAND
#define GEO_SHAPE_CHANGE_PROJECTION_COMMAND

#include <global.h>

#include <QtGui/QUndoCommand>
#include <kundo2command.h>

class MarbleMapShape;
//enum Projection {};

/// The undo / redo command for changing the projection
class MarbleMapShapeCommandChangeProjection : public KUndo2Command
{
public:
    /**
    * Command to change the projection that is used.
    *
    * @param shape the shape where we want to zoom
    * @param projection projection that is wanted
    * @param parent the parent command used for macro commands
     */
    MarbleMapShapeCommandChangeProjection(MarbleMapShape * shape, Marble::Projection projection, KUndo2Command *parent = 0);

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    MarbleMapShape *m_shape;
    Marble::Projection m_old_projection;
    Marble::Projection m_new_projection;
};


#endif
