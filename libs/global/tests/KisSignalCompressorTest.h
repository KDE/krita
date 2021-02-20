/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSIGNALCOMPRESSORTEST_H
#define KISSIGNALCOMPRESSORTEST_H

#include <QtTest>
#include <QObject>

class KisSignalCompressorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test();
    void testSlowHandlerPrecise();
    void testSlowHandlerAdditive();
    void testIdleChecks();
};

#endif // KISSIGNALCOMPRESSORTEST_H
