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

#ifndef FILTERREMOVECOMMAND_H
#define FILTERREMOVECOMMAND_H

#include <kundo2command.h>

class KoShape;
class KoFilterEffect;
class KoFilterEffectStack;

/// A command do remove a filter effect from a filter effect stack
class FilterRemoveCommand : public KUndo2Command
{
public:
    FilterRemoveCommand(int filterEffectIndex, KoFilterEffectStack *filterStack, KoShape *shape, KUndo2Command *parent = 0);
    ~FilterRemoveCommand();
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    KoFilterEffect *m_filterEffect;
    KoFilterEffectStack *m_filterStack;
    KoShape *m_shape;
    bool m_isRemoved;
    int m_filterEffectIndex;
};

#endif // FILTERREMOVECOMMAND_H
