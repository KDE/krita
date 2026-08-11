/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_INPUT_PROFILE_MANAGER_TEST_H
#define __KIS_INPUT_PROFILE_MANAGER_TEST_H

#include <simpletest.h>

class KisInputProfileManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testProfileCreation();
    void testShortcutConfigurationTouchGesture_data();
    void testShortcutConfigurationTouchGesture();

    void testShortcutConfigurationNativeGesture_data();
    void testShortcutConfigurationNativeGesture();
};

#endif /* __KIS_INPUT_PROFILE_MANAGER_TEST_H */
