/*
 *  SPDX-FileCopyrightText: 2026 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CUT_THROUGH_SHAPE_STRATEGY_TEST_H
#define CUT_THROUGH_SHAPE_STRATEGY_TEST_H

#include <QObject>

class CutThroughShapeStrategyTest : public QObject
{
    Q_OBJECT
private:
    void addRandom();

private Q_SLOTS:

    void WillShapeBeCutTest_data();
    void WillShapeBeCutTest();
};

#endif // CUT_THROUGH_SHAPE_STRATEGY_TEST_H
