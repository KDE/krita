/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TILE_DATA_POOLER_TEST_H
#define __KIS_TILE_DATA_POOLER_TEST_H

#include <QtTest>

class KisTileDataPoolerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCycles();
};

#endif /* __KIS_TILE_DATA_POOLER_TEST_H */
