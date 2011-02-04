/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef SETCLEFACTION_H
#define SETCLEFACTION_H

#include "AbstractMusicAction.h"
#include "../core/Clef.h"

class SetClefAction : public AbstractMusicAction
{
public:
    SetClefAction(MusicCore::Clef::ClefShape shape, int line, int octaveChange, SimpleEntryTool* tool);
    virtual void mousePress(MusicCore::Staff* staff, int bar, const QPointF& pos);
private:
    MusicCore::Clef::ClefShape m_shape;
    int m_line;
    int m_octaveChange;
};

#endif // SETCLEFACTION_H
