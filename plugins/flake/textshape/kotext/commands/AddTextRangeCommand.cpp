/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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
 * Boston, MA 02110-1301, USA.*/

#include "AddTextRangeCommand.h"

#include <klocalizedstring.h>
#include "TextDebug.h"

#include <KoTextRangeManager.h>
#include <KoTextRange.h>

AddTextRangeCommand::AddTextRangeCommand(KoTextRange * range, KUndo2Command *parent)
    : KUndo2Command(kundo2_noi18n("internal step"), parent)
    , m_range(range)
{
}

void AddTextRangeCommand::undo()
{
    KUndo2Command::undo();
    m_range->manager()->remove(m_range);
}

void AddTextRangeCommand::redo()
{
    KUndo2Command::redo();
    m_range->manager()->insert(m_range);
}


AddTextRangeCommand::~AddTextRangeCommand()
{
}
