/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TRANSFORMSTROKESTRATEGYTEST_H
#define TRANSFORMSTROKESTRATEGYTEST_H

#include <QObject>

class TransformStrokeStrategyTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testLod2();
    void testLod2Cancelled();
    void testLod2ContinueAndCancel();
    void testLod0();
    void testLod0Cancelled();
    void testLod0ContinueAndCancel();
};

#endif // TRANSFORMSTROKESTRATEGYTEST_H
