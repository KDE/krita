/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STROKE_STRATEGY_UNDO_COMMAND_BASED_TEST_H
#define __KIS_STROKE_STRATEGY_UNDO_COMMAND_BASED_TEST_H

#include <QtTest>

class KisStrokeStrategyUndoCommandBasedTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFinishedStroke();
    void testCancelledStroke();
    void stressTestSequentialCommands();
};

#endif /* __KIS_STROKE_STRATEGY_UNDO_COMMAND_BASED_TEST_H */
