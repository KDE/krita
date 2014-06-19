/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2014 Alexander Potashev <aspotashev@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kundo2magicstring.h"


KUndo2MagicString::KUndo2MagicString()
{
}

KUndo2MagicString::KUndo2MagicString(const QString &text)
    : m_text(text)
{
}

QString KUndo2MagicString::toString() const
{
    int cdpos = m_text.indexOf(QLatin1Char('\n'));
    return cdpos > 0 ? m_text.left(cdpos) : m_text;
}

QString KUndo2MagicString::toSecondaryString() const
{
    int cdpos = m_text.indexOf(QLatin1Char('\n'));
    return cdpos > 0 ? m_text.mid(cdpos + 1) : m_text;
}

bool KUndo2MagicString::isEmpty() const
{
    return m_text.isEmpty();
}

bool KUndo2MagicString::operator==(const KUndo2MagicString &rhs) const
{
    return m_text == rhs.m_text;
}

bool KUndo2MagicString::operator!=(const KUndo2MagicString &rhs) const
{
    return !(*this == rhs);
}
