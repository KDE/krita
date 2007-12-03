/* This file is part of the KDE project
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

#ifndef RECTANGLESHAPECONFIGCOMMAND_H
#define RECTANGLESHAPECONFIGCOMMAND_H

#include <QtGui/QUndoCommand>

class KoRectangleShape;

/// The undo / redo command for configuring a rectangle shape
class RectangleShapeConfigCommand : public QUndoCommand
{
public:
    /**
     * Configures a rectangle shape
     * @param Rectangle the rectangle shape to configure
     * @param cornerRadiusX the x corner radius
     * @param cornerRadiusY the y corner radius
     * @param parent the optional parent command
     */
    RectangleShapeConfigCommand( KoRectangleShape * rectangle, double cornerRadiusX, double cornerRadiusY, QUndoCommand *parent = 0 );
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();
private:
    KoRectangleShape * m_rectangle;
    double m_oldCornerRadiusX;
    double m_oldCornerRadiusY;
    double m_newCornerRadiusX;
    double m_newCornerRadiusY;
};

#endif // RECTANGLESHAPECONFIGCOMMAND_H

