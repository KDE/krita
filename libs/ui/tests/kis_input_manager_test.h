/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2026 Ayanami Kaine <personal@ayanamikaine.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_INPUT_MANAGER_TEST_H
#define __KIS_INPUT_MANAGER_TEST_H

#include <simpletest.h>

class KisInputManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSingleActionShortcut();
    void testStrokeShortcut();
    void testKeyEvents();
    void testReleaseUnnecessaryModifiers();
    void testMouseMoves();
    void testStrokeShortcutSwitchFallback();
    void testStrokeShortcutSwitchModes();
    void testStrokeShortcutModifierFirstMode();
    void testRunReadyShortcutSwitch();
    void testKeyedStrokeLifecycle();
    void testStrokeShortcutCleanup();

    void testTouchMoves_data();
    void testTouchMoves();

    void testTouchHoldPostponer_data();
    void testTouchHoldPostponer();

    void testTouchOverriddenByTablet_data();
    void testTouchOverriddenByTablet();

    void testIncrementalAverage();
};

#endif /* __KIS_INPUT_MANAGER_TEST_H */
