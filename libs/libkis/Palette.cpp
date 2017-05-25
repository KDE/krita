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

struct Palette::Private {
    KoColorSet *palette {0};
};

Palette::Palette(Resource *resource): d(new Private()) {
    d->palette = dynamic_cast<KoColorSet*>(resource->resource());
}

int Palette::columnCount()
{
    return d->palette->columnCount();
}

void Palette::setColumnCount(int columns)
{
    d->palette->setColumnCount(columns);
}

QString Palette::comment()
{
    return d->palette->comment();
}

QStringList Palette::groupNames()
{
    return d->palette->getGroupNames();
}

bool Palette::addGroup(QString name)
{
    return d->palette->addGroup(name);
}

bool Palette::removeGroup(QString name, bool keepColors)
{
    return d->palette->removeGroup(name, keepColors);
}

int Palette::colorsCountGroup(QString name)
{
    return d->palette->nColorsGroup(name);
}
