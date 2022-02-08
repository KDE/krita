/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBRUSHMODELTEST_H
#define KISBRUSHMODELTEST_H

#include <QObject>

class KisBrushModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testAutoBrush();
    void testPredefinedBrush();
    void testTextBrush();
};

#endif // KISBRUSHMODELTEST_H
