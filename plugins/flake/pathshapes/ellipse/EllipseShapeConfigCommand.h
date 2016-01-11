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

#ifndef ELLIPSESHAPECONFIGCOMMAND_H
#define ELLIPSESHAPECONFIGCOMMAND_H

#include "EllipseShape.h"
#include <kundo2command.h>

/// The undo / redo command for configuring an ellipse shape
class EllipseShapeConfigCommand : public KUndo2Command
{
public:
    /**
     * Configures an ellipse shape
     * @param ellipse the ellipse shape to configure
     * @param type the ellipse type
     * @param startAngle the start angle
     * @param endAngle the end angle
     * @param parent the optional parent command
     */
    EllipseShapeConfigCommand(EllipseShape *ellipse, EllipseShape::EllipseType type, qreal startAngle, qreal startEndAngle, KUndo2Command *parent = 0);
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();
private:
    EllipseShape *m_ellipse;
    EllipseShape::EllipseType m_oldType;
    qreal m_oldStartAngle;
    qreal m_oldEndAngle;
    EllipseShape::EllipseType m_newType;
    qreal m_newStartAngle;
    qreal m_newEndAngle;
};

#endif // ELLIPSESHAPECONFIGCOMMAND_H

