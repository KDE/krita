/*
 *  Copyright (c) 1999 Michael Koch    <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QString>
#include "kis_command.h"

KisCommand::KisCommand(KisUndoAdapter *adapter)
{
    m_name = "";
    m_undoAdapter = adapter;
}

KisCommand::KisCommand(const QString& name, KisUndoAdapter *adapter)
{
    m_name = name;
    m_undoAdapter = adapter;
}

KisCommand::~KisCommand()
{
}

QString KisCommand::name() const
{
    return m_name;
}

