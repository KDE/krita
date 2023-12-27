/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "Palette.h"
#include <KoColorSet.h>
#include <KisSwatch.h>
#include <KisSwatchGroup.h>
#include <ManagedColor.h>
#include <KisPaletteModel.h>

struct Palette::Private {
    KoColorSetSP palette {0};
};

Palette::Palette(Resource *resource): d(new Private()) {
    d->palette = resource->resource().dynamicCast<KoColorSet>();
}

Palette::~Palette()
{
    delete d;
}

int Palette::numberOfEntries() const
{
    if (!d->palette) return 0;
    return d->palette->colorCount();
}

int Palette::columnCount()
{
    if (!d->palette) return 0;
    return d->palette->columnCount();
}

void Palette::setColumnCount(int columns)
{
    if (d->palette && columns > 0)
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

QStringList Palette::groupNames() const
{
    if (!d->palette) return QStringList();
    return d->palette->swatchGroupNames();
}

void Palette::addGroup(QString name)
{
    if (!d->palette) return;
    d->palette->addGroup(name);
}

void Palette::removeGroup(QString name, bool keepColors)
{
    if (!d->palette) return;
    return d->palette->removeGroup(name, keepColors);
}

int Palette::colorsCountTotal()
{
    if (!d->palette) return 0;
    return d->palette->colorCount();
}

Swatch *Palette::colorSetEntryByIndex(int index)
{
    if (!d->palette || columnCount() == 0) {
        return new Swatch();
    }
    int col = index % columnCount();
    int row = (index - col) / columnCount();
    return new Swatch(d->palette->getColorGlobal(col, row));
}

Swatch *Palette::colorSetEntryFromGroup(int index, const QString &groupName)
{
    if (!d->palette || columnCount() == 0) {
        return new Swatch();
    }
    int row = index % columnCount();
    return new Swatch(d->palette->getSwatchFromGroup((index - row) / columnCount(), row, groupName));
}

void Palette::addEntry(Swatch entry, QString groupName)
{
    d->palette->addSwatch(entry.kisSwatch(), groupName);
}

void Palette::removeEntry(int index, const QString &/*groupName*/)
{
    int col = index % columnCount();
    int tmp = index;
    int row = (index - col) / columnCount();
    KisSwatchGroupSP groupFoundIn;
    Q_FOREACH(const QString &name, groupNames()) {
        KisSwatchGroupSP g = d->palette->getGroup(name);
        tmp -= g->rowCount() * columnCount();
        if (tmp < 0) {
            groupFoundIn = g;
            break;
        }
        row -= g->rowCount();

    }
    if (!groupFoundIn) { return; }
    d->palette->removeSwatch(col, row, groupFoundIn);
}

void Palette::changeGroupName(QString oldGroupName, QString newGroupName)
{
    d->palette->changeGroupName(oldGroupName, newGroupName);
}

void Palette::moveGroup(const QString &groupName, const QString &groupNameInsertBefore)
{
    return d->palette->moveGroup(groupName, groupNameInsertBefore);
}

bool Palette::save()
{
    return false;
}

KoColorSetSP Palette::colorSet()
{
    return d->palette;
}
