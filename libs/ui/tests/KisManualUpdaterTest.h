/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISMANUALUPDATERTEST_H
#define KISMANUALUPDATERTEST_H

#include <QObject>

class KisManualUpdaterTest : public QObject
{
    Q_OBJECT
public:
    explicit KisManualUpdaterTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testAvailableVersionIsHigher();
    void testAvailableVersionIsHigher_data();

    void testCheckForUpdate();
    void testCheckForUpdate_data();
};

#endif // KISMANUALUPDATERTEST_H
