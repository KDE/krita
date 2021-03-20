/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_RAW_TEST_H_
#define _KIS_RAW_TEST_H_

#include <simpletest.h>

class KisRawTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testImportFromWriteonly();
    void testImportIncorrectFormat();
};

#endif // _KIS_RAW_TEST_H_

