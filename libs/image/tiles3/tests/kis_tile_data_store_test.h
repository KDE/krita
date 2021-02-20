/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TILE_DATA_STORE_TEST_H
#define KIS_TILE_DATA_STORE_TEST_H

#include <QtTest>

class KisTileDataStoreTest : public QObject
{
    Q_OBJECT

private:

private Q_SLOTS:
    void testClockIterator();
    void testLeaks();
    void testSwapping();
};

#endif /* KIS_TILE_DATA_STORE_TEST_H */

