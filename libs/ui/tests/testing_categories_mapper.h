/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
