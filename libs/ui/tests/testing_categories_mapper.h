/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __TESTING_CATEGORIES_MAPPER_H
#define __TESTING_CATEGORIES_MAPPER_H

#include "kis_categories_mapper.h"
#include <QTest>

struct QStringConverter {
    QString operator() (const QString &entry) {
        return entry;
    }
};
typedef KisCategoriesMapper<QString, QStringConverter> TestingBaseMapper;

class TestingMapper : public TestingBaseMapper
{
    Q_OBJECT
public:
    TestingMapper()
        : insertCounter(0),
          removeCounter(0)
    {
        connect(this, SIGNAL(rowChanged(int)), SLOT(slotRowChanged(int)));
        connect(this, SIGNAL(beginInsertRow(int)), SLOT(slotBeginInsertRow(int)));
        connect(this, SIGNAL(endInsertRow()), SLOT(slotEndInsertRow()));
        connect(this, SIGNAL(beginRemoveRow(int)), SLOT(slotBeginRemoveRow(int)));
        connect(this, SIGNAL(endRemoveRow()), SLOT(slotEndRemoveRow()));
    }

    void checkInsertedCorrectly(int num = 1) {
        QCOMPARE(insertCounter, 2 * num);
        insertCounter = 0;
    }

    void checkRemovedCorrectly(int num = 1) {
        QCOMPARE(removeCounter, 2 * num);
        removeCounter = 0;
    }

    void checkRowChangedIndices(const QVector<int> indices) {
        QCOMPARE(indices.size(), rowChangedIndices.size());

        Q_FOREACH (int index, rowChangedIndices) {
            QVERIFY(indices.contains(index));
        }

        rowChangedIndices.clear();
    }

    using TestingBaseMapper::testingGetItems;

protected Q_SLOTS:
    void slotRowChanged(int row) {
        rowChangedIndices.append(row);
    }

    void slotBeginInsertRow(int row) {
        Q_UNUSED(row);
        insertCounter++;
    }

    void slotEndInsertRow() {
        insertCounter++;
    }

    void slotBeginRemoveRow(int row) {
        Q_UNUSED(row);
        removeCounter++;
    }

    void slotEndRemoveRow() {
        removeCounter++;
    }

private:
    int insertCounter;
    int removeCounter;
    QVector<int> rowChangedIndices;
};

#endif /* __TESTING_CATEGORIES_MAPPER_H */
