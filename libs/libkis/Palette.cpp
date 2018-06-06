/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "Palette.h"
#include <KoColorSet.h>
#include <ManagedColor.h>

struct Palette::Private {
    KoColorSet *palette {0};
};

Palette::Palette(Resource *resource): d(new Private()) {
    d->palette = dynamic_cast<KoColorSet*>(resource->resource());
}

Palette::~Palette()
{
    delete d;
}

int Palette::numberOfEntries() const
{
    if (!d->palette) return 0;
    return d->palette->nColors();
}

int Palette::columnCount()
{
    if (!d->palette) return 0;
    return d->palette->columnCount();
}

void Palette::setColumnCount(int columns)
{
    if (d->palette)
        d->palette->setColumnCount(columns);
}

QString Palette::comment()
{
    if (!d->palette) return "";
    return d->palette->comment();
}

void Palette::setComment(QString comment)
{
    if (!d->palette) return;
    return d->palette->setComment(comment);
}

QStringList Palette::groupNames()
{
    if (!d->palette) return QStringList();
    return d->palette->getGroupNames();
}

bool Palette::addGroup(QString name)
{
    if (!d->palette) return false;
    return d->palette->addGroup(name);
}

bool Palette::removeGroup(QString name, bool keepColors)
{
    if (!d->palette) return false;
    return d->palette->removeGroup(name, keepColors);
}

int Palette::colorsCountTotal()
{
    if (!d->palette) return 0;
    return d->palette->nColors();
}

int Palette::colorsCountGroup(QString name)
{
    if (!d->palette) return 0;
    return d->palette->nColorsGroup(name);
}

KoColorSetEntry Palette::colorSetEntryByIndex(int index)
{
    if (!d->palette) return KoColorSetEntry();
    return d->palette->getColorGlobal(index);
}

KoColorSetEntry Palette::colorSetEntryFromGroup(int index, const QString &groupName)
{
    if (!d->palette) return KoColorSetEntry();

    return d->palette->getColorGroup(index, groupName);
}

ManagedColor *Palette::colorForEntry(KoColorSetEntry entry)
{
    if (!d->palette) return 0;
    ManagedColor *color = new ManagedColor(entry.color());
    return color;
}

void Palette::addEntry(KoColorSetEntry entry, QString groupName)
{
    d->palette->add(entry, groupName);
}

void Palette::removeEntry(int index, const QString &groupName)
{
    d->palette->removeAt(index, groupName);
}

void Palette::insertEntry(int index, KoColorSetEntry entry, QString groupName)
{
    d->palette->insertBefore(entry, index, groupName);
}

bool Palette::editEntry(int index, KoColorSetEntry entry, QString groupName)
{
    return d->palette->changeColorSetEntry(entry, groupName, index);
}

bool Palette::changeGroupName(QString oldGroupName, QString newGroupName)
{
    return d->palette->changeGroupName(oldGroupName, newGroupName);
}

bool Palette::moveGroup(const QString &groupName, const QString &groupNameInsertBefore)
{
    return d->palette->moveGroup(groupName, groupNameInsertBefore);
}

bool Palette::save()
{
    if (d->palette->filename().size()>0) {
        return d->palette->save();
    }
    //if there's no filename the palette proly doesn't even exist...
    return false;
}

KoColorSet *Palette::colorSet()
{
    return d->palette;
}
