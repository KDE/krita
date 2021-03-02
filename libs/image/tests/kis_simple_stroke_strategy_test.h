/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SIMPLE_STROKE_STRATEGY_TEST_H
#define __KIS_SIMPLE_STROKE_STRATEGY_TEST_H

#include <simpletest.h>

class KisSimpleStrokeStrategyTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFinish();
    void testCancel();
};

#endif /* __KIS_SIMPLE_STROKE_STRATEGY_TEST_H */
