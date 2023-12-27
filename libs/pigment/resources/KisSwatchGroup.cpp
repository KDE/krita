/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2005..2022 Halla Rempt <halla@valdyas.org>
    SPDX-FileCopyrightText: 2016 L. E. Segovia <amy@amyspark.me>
    SPDX-FileCopyrightText: 2018 Michael Zhou <simerixh@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later

 */

#include "KisSwatchGroup.h"

int KisSwatchGroup::DEFAULT_COLUMN_COUNT = 16;
int KisSwatchGroup::DEFAULT_ROW_COUNT = 20;

struct KisSwatchGroup::Private {
    typedef QMap<int, KisSwatch> Column;

    QString name {QString()};
    QVector<Column> colorMatrix {DEFAULT_COLUMN_COUNT};
    int colorCount {0};
    int rowCount {DEFAULT_ROW_COUNT};
};


KisSwatchGroup::KisSwatchGroup()
    : d(new Private)
{ }

KisSwatchGroup::~KisSwatchGroup()
{ }

KisSwatchGroup::KisSwatchGroup(const KisSwatchGroup &rhs)
    : d(new Private(*rhs.d))
{ }

KisSwatchGroup &KisSwatchGroup::operator =(const KisSwatchGroup &rhs)
{
    if (&rhs == this) {
        return *this;
    }
    *d = *rhs.d;
    return *this;
}

void KisSwatchGroup::setSwatch(const KisSwatch &e, int column, int row)
{
    Q_ASSERT(column < d->colorMatrix.size() && column >= 0 && row >= 0);

    if (row + 1 >= d->rowCount) {
        setRowCount(row + 1);
    }
    if (!checkSwatchExists(column, row)) {
        d->colorCount++;
    }
    d->colorMatrix[column][row] = e;
}

bool KisSwatchGroup::checkSwatchExists(int column, int row) const
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

bool KisSwatchGroup::removeSwatch(int column, int row)
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
    KIS_SAFE_ASSERT_RECOVER_RETURN(columnCount >= 1);


    // Move 'removed' swatches into new row
    QVector <KisSwatch> movedSwatches;

    for (int r = 0; r < rowCount(); r++ ) {
        for (int c = 0; c < d->colorMatrix.size(); c++ ) {

            if (c >= columnCount && checkSwatchExists(c, r)) {
                movedSwatches.push_back(getSwatch(c, r));
            }
        }
    }
    if ( !movedSwatches.isEmpty()) {
        for (int i = 0; i< movedSwatches.size(); i++) {
            int r = (i/columnCount) + d->rowCount;
            int c = (i%columnCount);
            d->colorMatrix[c][r] = movedSwatches.at(i);
        }
        d->rowCount += (movedSwatches.size()/columnCount);
        if (movedSwatches.size()%columnCount > 0) {
            d->rowCount += 1;
        }
    }

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

KisSwatch KisSwatchGroup::getSwatch(int column, int row) const
{
    // This is perfectly normal when Krita gets initialized, so it needs an if, not an assert.
    // Getting -1, -1 is not a coding error.
    if (row < 0 || column < 0) return KisSwatch();

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(column >= 0 && column < d->colorMatrix.size(), KisSwatch());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(row >= 0 && row < d->rowCount, KisSwatch());

    return d->colorMatrix[column][row];
}

QPair<int, int> KisSwatchGroup::addSwatch(const KisSwatch &swatch)
{
    if (columnCount() == 0) {
        setColumnCount(DEFAULT_COLUMN_COUNT);
    }

    int y = 0;
    int x = 0;
    while (checkSwatchExists(x, y))
    {
        if(++x == d->colorMatrix.size())
        {
            x = 0;
            ++y;
        }
    }
    // clear color metadata for now.
    swatch.color().clearMetadata();

    setSwatch(swatch, x, y);

    return QPair<int, int> (x, y);
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
