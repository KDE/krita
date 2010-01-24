/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#include "PartGroup.h"
#include "Sheet.h"

namespace MusicCore {

class PartGroup::Private
{
public:
    GroupSymbol symbol;
    QString name;
    QString shortName;
    int firstPart;
    int lastPart;
    bool commonBarLines;
};

PartGroup::PartGroup(Sheet* sheet, int firstPart, int lastPart) : QObject(sheet), d(new Private)
{
    d->symbol = DefaultSymbol;
    d->firstPart = firstPart;
    d->lastPart = lastPart;
    d->commonBarLines = true;
}

PartGroup::~PartGroup()
{
    delete d;
}

Sheet* PartGroup::sheet()
{
    return qobject_cast<Sheet*>(parent());
}

int PartGroup::firstPart() const
{
    return d->firstPart;
}

void PartGroup::setFirstPart(int index)
{
    Q_ASSERT( index >= 0 && index < sheet()->partCount() );
    if (d->firstPart == index) return;
    d->firstPart = index;
    emit firstPartChanged(index);
}

int PartGroup::lastPart() const
{
    return d->lastPart;
}

void PartGroup::setLastPart(int index)
{
    Q_ASSERT( index >= 0 && index < sheet()->partCount() );
    if (d->lastPart == index) return;
    d->lastPart = index;
    emit lastPartChanged(index);
}

QString PartGroup::name() const
{
    return d->name;
}

void PartGroup::setName(const QString &name)
{
    if (d->name == name) return;
    d->name = name;
    emit nameChanged(name);
    if (d->shortName.isNull()) emit shortNameChanged(name);
}

QString PartGroup::shortName(bool useFull) const
{
    if (d->shortName.isNull() && useFull) {
        return d->name;
    } else {
        return d->shortName;
    }
}

void PartGroup::setShortName(const QString& shortName)
{
    if (d->shortName == shortName) return;
    d->shortName = shortName;
    emit shortNameChanged(shortName);
}

PartGroup::GroupSymbol PartGroup::symbol() const
{
    return d->symbol;
}

void PartGroup::setSymbol(GroupSymbol symbol)
{
    if (d->symbol == symbol) return;
    d->symbol = symbol;
    emit symbolChanged(symbol);
}

bool PartGroup::commonBarLines() const
{
    return d->commonBarLines;
}

void PartGroup::setCommonBarLines(bool commonBarLines)
{
    if (d->commonBarLines == commonBarLines) return;
    d->commonBarLines = commonBarLines;
    emit commonBarLinesChanged(commonBarLines);
}

} // namespace MusicCore

#include <PartGroup.moc>
