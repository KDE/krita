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

#ifndef STARSHAPECONFIGCOMMAND_H
#define STARSHAPECONFIGCOMMAND_H

#include <QtGui/QUndoCommand>

class StarShape;

/// The undo / redo command for configuring a star shape
class StarShapeConfigCommand : public QUndoCommand
{
public:
    /**
     * Configures a star shape
     * @param star the star shape to configure
     * @param cornerCount the number of corners to set
     * @param innerRadius the inner radius
     * @param outerRadius the outer radius
     * @param convex indicates whether the star is convex or not
     * @param parent the optional parent command
     */
    StarShapeConfigCommand(StarShape *star, uint cornerCount, qreal innerRadius, qreal outerRadius, bool convex, QUndoCommand *parent = 0);
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();
private:
    StarShape *m_star;
    uint m_oldCornerCount;
    qreal m_oldInnerRadius;
    qreal m_oldOuterRadius;
    bool m_oldConvex;
    uint m_newCornerCount;
    qreal m_newInnerRadius;
    qreal m_newOuterRadius;
    bool m_newConvex;
};

#endif // STARSHAPECONFIGCOMMAND_H

