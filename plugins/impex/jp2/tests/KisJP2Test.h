/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_JP2_TEST_H_
#define _KIS_JP2_TEST_H_

#include <QtTest>

class KisJP2Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testImportFromWriteonly();
    void testExportToReadonly();
    void testImportIncorrectFormat();
};

#endif // _KIS_JP2_TEST_H_

