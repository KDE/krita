/*
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_HEIGHTMAP_TEST_H_
#define _KIS_HEIGHTMAP_TEST_H_

#include <QtTest>

class KisHeightmapTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFiles();

    void testImportFromWriteonly();
    void testExportToReadonly();
    void testImportIncorrectFormat();
};

#endif // _KIS_HEIGHTMAP_TEST_H_
