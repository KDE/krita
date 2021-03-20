/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Alexander Potashev <aspotashev@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
