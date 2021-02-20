/*
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_XCF_TEST_H_
#define _KIS_XCF_TEST_H_

#include <QtTest>

class KisXCFTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFiles();

    void testImportFromWriteonly();
    // You can't export to xcf
    /* void testExportToReadonly(); */
    void testImportIncorrectFormat();
};

#endif
