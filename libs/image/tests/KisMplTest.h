/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISMPLTEST_H
#define KISMPLTEST_H

#include <QtTest/QtTest>

class KisMplTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

#endif // KISMPLTEST_H
