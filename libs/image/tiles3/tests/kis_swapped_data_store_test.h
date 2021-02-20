/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SWAPPED_DATA_STORE_TEST_H
#define KIS_SWAPPED_DATA_STORE_TEST_H

#include <QtTest>

class KisTileData;
class KisSwappedDataStore;

class KisSwappedDataStoreTest : public QObject
{
    Q_OBJECT

private:
    void processTileData(qint32 column, KisTileData *td, KisSwappedDataStore &store);

private Q_SLOTS:
    void testRoundTrip();
    void testRandomAccess();

};

#endif /* KIS_SWAPPED_DATA_STORE_TEST_H */

