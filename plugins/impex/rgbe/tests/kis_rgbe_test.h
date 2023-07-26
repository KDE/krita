/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_RGBE_TEST_H
#define _KIS_RGBE_TEST_H

#include <simpletest.h>

class KisRGBETest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFiles();
    void testHDR();
    void testSaveRgbaColorSpace();
    void testImportIncorrectFormat();

#ifndef Q_OS_WIN
private Q_SLOTS:
    void testImportFromWriteonly();
    void testExportToReadonly();
#endif

};

#endif // _KIS_RGBE_TEST_H
