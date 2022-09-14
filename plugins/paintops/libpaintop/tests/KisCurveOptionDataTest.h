/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCURVEOPTIONDATATEST_H
#define KISCURVEOPTIONDATATEST_H

#include <simpletest.h>

class KisCurveOptionDataTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCurveOptionData();
    void testSerializeDisabledSensors();
    void testSerializeNoSensors();
};

#endif // KISCURVEOPTIONDATATEST_H
