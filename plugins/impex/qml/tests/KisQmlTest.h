/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_QML_TEST_H_
#define _KIS_QML_TEST_H_

#include <simpletest.h>

class KisQmlTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testExportToReadonly();
};

#endif // _KIS_QML_TEST_H_

