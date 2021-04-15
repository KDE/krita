/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LOW_MEMORY_TESTS_H
#define __KIS_LOW_MEMORY_TESTS_H

#include <simpletest.h>

class KisLowMemoryTests : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void readWriteOnSharedTiles();
    void hangingTilesTest();
};

#endif /* __KIS_LOW_MEMORY_TESTS_H */
