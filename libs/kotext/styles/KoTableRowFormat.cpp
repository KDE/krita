/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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

#include "KoTableRowFormat.h"

#include <QBrush>

KoTableRowFormat::KoTableRowFormat() : KoTableFormat(RowFormat)
{
}

void KoTableRowFormat::setBackground(const QBrush &background)
{
    setProperty(Background, background);
}

QBrush KoTableRowFormat::background() const
{
    return brushProperty(Background);
}

void KoTableRowFormat::setMinimumHeight(qreal minimumHeight)
{
    setProperty(MinimumHeight, minimumHeight);
}

qreal KoTableRowFormat::minimumHeight() const
{
    return doubleProperty(MinimumHeight);
}

void KoTableRowFormat::setKeepTogether(bool keepTogether)
{
    setProperty(KeepTogether, keepTogether);
}

bool KoTableRowFormat::keepTogether() const
{
    return boolProperty(KeepTogether);
}

void KoTableRowFormat::setBreakAfter(bool breakAfter)
{
    setProperty(BreakAfter, breakAfter);
}

bool KoTableRowFormat::breakAfter() const
{
    return boolProperty(BreakAfter);
}

void KoTableRowFormat::setBreakBefore(bool breakBefore)
{
    setProperty(BreakBefore, breakBefore);
}

bool KoTableRowFormat::breakBefore() const
{
    return boolProperty(BreakBefore);
}

bool KoTableRowFormat::isValid() const
{
    return isRowFormat();
}

