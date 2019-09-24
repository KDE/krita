/*  This file is part of the KDE project
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
    Copyright (c) 2016 L. E. Segovia <leo.segovia@siggraph.org>
    Copyright (c) 2018 Michael Zhou <simerixh@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 */

#include "KisSwatchGroup.h"

struct KisSwatchGroup::Private {
    typedef QMap<int, KisSwatch> Column;

    static int DEFAULT_COLUMN_COUNT;
    static int DEFAULT_ROW_COUNT;

    QString name {QString()};
    QVector<Column> colorMatrix {DEFAULT_COLUMN_COUNT};
    int colorCount {0};
    int rowCount {DEFAULT_ROW_COUNT};
};

int KisSwatchGroup::Private::DEFAULT_COLUMN_COUNT = 16;
int KisSwatchGroup::Private::DEFAULT_ROW_COUNT = 20;

KisSwatchGroup::KisSwatchGroup()
    : d(new Private)
{ }

KisSwatchGroup::~KisSwatchGroup() = default;

KisSwatchGroup::KisSwatchGroup(const KisSwatchGroup &rhs)
    : d(new Private(*rhs.d))
{ }

KisSwatchGroup &KisSwatchGroup::operator =(const KisSwatchGroup &rhs)
{
    if (&rhs == this) {
        return *this;
    }
    d.reset(new Private(*rhs.d));
    return *this;
}

void KisSwatchGroup::setEntry(const KisSwatch &e, int column, int row)
{
    Q_ASSERT(column < d->colorMatrix.size() && column >= 0 && row >= 0);
    if (row >= d->rowCount) {
        setRowCount(row + 1);
    }
    if (!checkEntry(column, row)) {
        d->colorCount++;
    }
    d->colorMatrix[column][row] = e;
}

bool KisSwatchGroup::checkEntry(int column, int row) const
{
    if (row >= d->rowCount) {
        return false;
    }

    if (column >= d->colorMatrix.size()){
        return false;
    }

    if (column < 0) {
        return false;
    }

    if (!d->colorMatrix[column].contains(row)) {
        return false;
    }

    if (!d->colorMatrix[column][row].isValid()) {
        return false;
    }

    return true;
}

bool KisSwatchGroup::removeEntry(int column, int row)
{
    if (d->colorCount == 0) {
        return false;
    }

    if (row >= d->rowCount || column >= d->colorMatrix.size() || column < 0) {
        return false;
    }

    // QMap::remove returns 1 if key found else 0
    if (d->colorMatrix[column].remove(row)) {
        d->colorCount -= 1;
        return true;
    } else {
        return false;
    }
}

void KisSwatchGroup::setColumnCount(int columnCount)
{
    Q_ASSERT(columnCount >= 0);

    if (columnCount < d->colorMatrix.size()) {
        int newColorCount = 0;
        for (int i = 0; i < columnCount; i++ ) {
            newColorCount += d->colorMatrix[i].size();
        }
        d->colorCount = newColorCount;
    }
    d->colorMatrix.resize(columnCount);
}

int KisSwatchGroup::columnCount() const {
    return d->colorMatrix.size();
}

KisSwatch KisSwatchGroup::getEntry(int column, int row) const
{
    Q_ASSERT(checkEntry(column, row));
    return d->colorMatrix[column][row];
}

void KisSwatchGroup::addEntry(const KisSwatch &e)
{
    if (columnCount() == 0) {
        setColumnCount(Private::DEFAULT_COLUMN_COUNT);
    }

    int y = 0;
    int x = 0;
    while(checkEntry(x, y))
    {
        if(++x == d->colorMatrix.size())
        {
            x = 0;
            ++y;
        }
    }
    setEntry(e, x, y);
}

void KisSwatchGroup::clear()
{
    d->colorMatrix.clear();
}

void KisSwatchGroup::setRowCount(int newRowCount)
{
    d->rowCount = newRowCount;
    for (Private::Column &c : d->colorMatrix) {
        for (int k : c.keys()) {
            if (k >= newRowCount) {
                c.remove(k);
                d->colorCount--;
            }
        }
    }
}

int KisSwatchGroup::rowCount() const
{
    return d->rowCount;
}

int KisSwatchGroup::colorCount() const
{
    return d->colorCount;
}

QList<KisSwatchGroup::SwatchInfo> KisSwatchGroup::infoList() const
{
    QList<SwatchInfo> res;
    int column = 0;
    for (const Private::Column &c : d->colorMatrix) {
        int i = 0;
        for (const KisSwatch &s : c.values()) {
            SwatchInfo info = {d->name, s, c.keys()[i++], column};
            res.append(info);
        }
        column++;
    }
    return res;
}

void KisSwatchGroup::setName(const QString &name)
{
    d->name = name;
}

QString KisSwatchGroup::name() const
{
    return d->name;
}
