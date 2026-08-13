/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisInputProfileManagerTest.h"

#include <simpletest.h>
#include <testutil.h>

#include <KisMpl.h>

#include <kis_debug.h>
#include <kis_config.h>

#include <input/kis_input_profile_manager.h>
#include <input/kis_input_profile.h>
#include <input/kis_shortcut_configuration.h>
#include <input/kis_abstract_input_action.h>
#include <input/kis_tool_invocation_action.h>

// for native gestures migration testing
#include <input/KisInputProfileMigrator.h>
#include <input/kis_zoom_and_rotate_action.h>


void KisInputProfileManagerTest::testProfileCreation()
{
    auto *profileManager = KisInputProfileManager::instance();

    auto *profile = profileManager->addProfile("Testing Profile");
    QCOMPARE(profileManager->profileNames(), {"Testing Profile"});

    profileManager->setCurrentProfile(profile);
    QCOMPARE(profileManager->currentProfile(), profile);
}

void KisInputProfileManagerTest::testLoadProfileV6()
{
    auto *profileManager = KisInputProfileManager::instance();

    const QString profileFileName = TestUtil::fetchDataFileLazy("krita_default_input_profile_v6.profile");

    ProfileEntry profileEntry;
    profileEntry.fullpath = profileFileName;

    {
        KConfig config(profileFileName, KConfig::SimpleConfig);
        profileEntry.version = config.group("General").readEntry("version", 0);
        profileEntry.name = config.group("General").readEntry("name");
    }

    auto *profile = profileManager->loadProfileWithMigration(profileEntry);

    QVERIFY(profile);
    QCOMPARE(profile->name(), "Test V6 Profile");

    QList< KisShortcutConfiguration* > shortcuts = profile->allShortcuts();

    KisShortcutConfiguration *foundNativeGestureShortcut = nullptr;
    int numInvalidNativeGestureShortcuts = 0;
    int totalNativeGestureShortcuts = 0;

    Q_FOREACH(KisShortcutConfiguration *shortcut, shortcuts) {
        if (shortcut->type() == KisShortcutConfiguration::NativeGestureType) {
            if (shortcut->nativeGesture() == KisShortcutConfiguration::PinchGesture) {
                totalNativeGestureShortcuts++;
                foundNativeGestureShortcut = shortcut;
            }
            if (shortcut->nativeGesture() > KisShortcutConfiguration::PinchGesture &&
                shortcut->nativeGesture() < KisShortcutConfiguration::SmartZoomGesture) {
                numInvalidNativeGestureShortcuts++;
            }
        }
    }

    if (foundNativeGestureShortcut) {
        QCOMPARE(foundNativeGestureShortcut->action()->id(), "Zoom and Rotate Canvas");
        QCOMPARE(foundNativeGestureShortcut->mode(), KisZoomAndRotateAction::PanAndZoomAndRotateMode);

    }

    QCOMPARE(totalNativeGestureShortcuts, 1);
    QCOMPARE(numInvalidNativeGestureShortcuts, 0);

    profileManager->removeProfile(profile->name());
}

Q_DECLARE_METATYPE(KisShortcutConfiguration::TouchGestureAction)

void KisInputProfileManagerTest::testShortcutConfigurationTouchGesture_data()
{
    QTest::addColumn<KisShortcutConfiguration::TouchGestureAction>("touchGestureAction");
    QTest::addColumn<QString>("expectedSerializedString");

    QTest::addRow("OneFingerTap") << KisShortcutConfiguration::OneFingerTap << "{3;4;[];0;0;1}";
    QTest::addRow("TwoFingerTap") << KisShortcutConfiguration::TwoFingerTap << "{3;4;[];0;0;2}";
    QTest::addRow("ThreeFingerTap") << KisShortcutConfiguration::ThreeFingerTap << "{3;4;[];0;0;3}";
    QTest::addRow("FourFingerTap") << KisShortcutConfiguration::FourFingerTap << "{3;4;[];0;0;4}";
    QTest::addRow("FiveFingerTap") << KisShortcutConfiguration::FiveFingerTap << "{3;4;[];0;0;5}";
    QTest::addRow("OneFingerDrag") << KisShortcutConfiguration::OneFingerDrag << "{3;4;[];0;0;6}";
    QTest::addRow("TwoFingerDrag") << KisShortcutConfiguration::TwoFingerDrag << "{3;4;[];0;0;7}";
    QTest::addRow("ThreeFingerDrag") << KisShortcutConfiguration::ThreeFingerDrag << "{3;4;[];0;0;8}";
    QTest::addRow("FourFingerDrag") << KisShortcutConfiguration::FourFingerDrag << "{3;4;[];0;0;9}";
    QTest::addRow("FiveFingerDrag") << KisShortcutConfiguration::FiveFingerDrag << "{3;4;[];0;0;a}";
    QTest::addRow("OneFingerHold") << KisShortcutConfiguration::OneFingerHold << "{3;4;[];0;0;b}";
}

void KisInputProfileManagerTest::testShortcutConfigurationTouchGesture()
{
    QFETCH(KisShortcutConfiguration::TouchGestureAction, touchGestureAction);
    QFETCH(QString, expectedSerializedString);

    auto *profileManager = KisInputProfileManager::instance();
    auto actions = profileManager->actions();

    const auto it = std::find_if(actions.begin(),
                                 actions.end(),
                                 kismpl::mem_equal_to(&KisAbstractInputAction::id, "Tool Invocation"));
    QVERIFY(it != actions.end());

    KisAbstractInputAction *action = *it;
    QVERIFY(action->shortcutIndexes().values().contains(KisToolInvocationAction::LineToolShortcut));

    KisShortcutConfiguration config;
    config.setType(KisShortcutConfiguration::TouchGestureType);
    config.setTouchGesture(touchGestureAction);
    config.setAction(action);
    config.setMode(KisToolInvocationAction::LineToolShortcut);

    // verify serialized string

    const QString buffer = config.serialize();
    QCOMPARE(buffer, expectedSerializedString);

    // verify round-trip loading

    KisShortcutConfiguration configLoaded;
    configLoaded.setAction(action);
    configLoaded.unserialize(buffer);

    QCOMPARE(configLoaded, config);
    QCOMPARE(configLoaded.type(), config.type());
    QCOMPARE(configLoaded.touchGesture(), config.touchGesture());

    QCOMPARE(configLoaded.action(), config.action());
    QCOMPARE(configLoaded.mode(), config.mode());
}

Q_DECLARE_METATYPE(KisShortcutConfiguration::NativeGestureAction)

void KisInputProfileManagerTest::testShortcutConfigurationNativeGesture_data()
{
    QTest::addColumn<KisShortcutConfiguration::NativeGestureAction>("nativeGestureAction");
    QTest::addColumn<QString>("expectedSerializedString");

    QTest::addRow("PinchGesture") << KisShortcutConfiguration::PinchGesture << "{3;5;[];0;0;1}";
    QTest::addRow("SmartZoomGesture") << KisShortcutConfiguration::SmartZoomGesture << "{3;5;[];0;0;4}";
}

void KisInputProfileManagerTest::testShortcutConfigurationNativeGesture()
{
    QFETCH(KisShortcutConfiguration::NativeGestureAction, nativeGestureAction);
    QFETCH(QString, expectedSerializedString);

    auto *profileManager = KisInputProfileManager::instance();
    auto actions = profileManager->actions();

    const auto it = std::find_if(actions.begin(),
                                 actions.end(),
                                 kismpl::mem_equal_to(&KisAbstractInputAction::id, "Tool Invocation"));
    QVERIFY(it != actions.end());

    KisAbstractInputAction *action = *it;
    QVERIFY(action->shortcutIndexes().values().contains(KisToolInvocationAction::LineToolShortcut));

    KisShortcutConfiguration config;
    config.setType(KisShortcutConfiguration::NativeGestureType);
    config.setNativeGesture(nativeGestureAction);
    config.setAction(action);
    config.setMode(KisToolInvocationAction::LineToolShortcut);

    // verify serialized string

    const QString buffer = config.serialize();
    QCOMPARE(buffer, expectedSerializedString);

    // verify round-trip loading

    KisShortcutConfiguration configLoaded;
    configLoaded.setAction(action);
    configLoaded.unserialize(buffer);

    QCOMPARE(configLoaded, config);
    QCOMPARE(configLoaded.type(), config.type());
    QCOMPARE(configLoaded.nativeGesture(), config.nativeGesture());

    QCOMPARE(configLoaded.action(), config.action());
    QCOMPARE(configLoaded.mode(), config.mode());
}

SIMPLE_TEST_MAIN(KisInputProfileManagerTest)
