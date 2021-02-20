/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISAPPIMAGEUPDATERTEST_H
#define KISAPPIMAGEUPDATERTEST_H

#include <QObject>

class KisAppimageUpdaterTest : public QObject
{
    Q_OBJECT
public:
    explicit KisAppimageUpdaterTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testCheckForUpdate();
    void testCheckForUpdate_data();
    void testDoUpdate();
    void testDoUpdate_data();
};

#endif // KISAPPIMAGEUPDATERTEST_H
