/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTKISSWATCHGROUP_H
#define TESTKISSWATCHGROUP_H

#include <QHash>
#include <QPair>

#include <KisSwatchGroup.h>

class TestKisSwatchGroup : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAddingOneEntry();
    void testAddingMultipleEntries();
    void testReplaceEntries();
    void testRemoveEntries();
    void testChangeColumnNumber();
    void testAddEntry();
private:
    KisSwatchGroup g;
    QHash<QPair<int, int>, KisSwatch> testSwatches;
};


#endif /* TESTKISSWATCHGROUP_H */
