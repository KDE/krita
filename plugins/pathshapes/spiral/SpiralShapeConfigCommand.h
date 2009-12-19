/* This file is part of the KDE project
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
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

#ifndef SPIRALSHAPECONFIGCOMMAND_H
#define SPIRALSHAPECONFIGCOMMAND_H

#include "SpiralShape.h"
#include <QtGui/QUndoCommand>

/// The undo / redo command for configuring a spiral shape
class SpiralShapeConfigCommand : public QUndoCommand
{
public:
    /**
     * Configures an spiral shape
     * @param spiral the spiral shape to configure
     * @param type the spiral type
     * @param fade the fade parameter
     * @param parent the optional parent command
     */
    SpiralShapeConfigCommand(SpiralShape *spiral, SpiralShape::SpiralType type, bool clockWise, qreal fade, QUndoCommand *parent = 0);
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();
private:
    SpiralShape *m_spiral;
    SpiralShape::SpiralType m_oldType;
    bool m_oldClockWise;
    qreal m_oldFade;
    SpiralShape::SpiralType m_newType;
    bool m_newClockWise;
    qreal m_newFade;
};

#endif // SPIRALSHAPECONFIGCOMMAND_H

