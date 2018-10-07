/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef FILTERSTACKSETCOMMAND_H
#define FILTERSTACKSETCOMMAND_H

#include <kundo2command.h>

class KoFilterEffectStack;
class KoShape;

/// Command to set a filter stack on a shape
class FilterStackSetCommand : public KUndo2Command
{
public:
    FilterStackSetCommand(KoFilterEffectStack *newStack, KoShape *shape, KUndo2Command *parent = 0);
    ~FilterStackSetCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    KoFilterEffectStack *m_newFilterStack;
    KoFilterEffectStack *m_oldFilterStack;
    KoShape *m_shape;
};

#endif // FILTERSTACKSETCOMMAND_H
