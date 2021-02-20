/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CSV_TEST_H_
#define _KIS_CSV_TEST_H_

#include <QtTest>

class KisCsvTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFiles();
    void testImportFromWriteonly();
    void testExportToReadonly();
    void testImportIncorrectFormat();
};

#endif
