/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPERSTROKERANDOMSOURCETEST_H
#define KISPERSTROKERANDOMSOURCETEST_H

#include <QtTest>

class KisPerStrokeRandomSourceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIndependent();
    void testDependent();
    void testDifferentKeys();
};

#endif // KISPERSTROKERANDOMSOURCETEST_H
