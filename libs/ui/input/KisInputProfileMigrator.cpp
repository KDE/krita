/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisInputProfileMigrator.h"

#include <QDebug>

#include <KisMpl.h>

#include <KConfig>
#include <KConfigGroup>
#include <KoResourcePaths.h>

#include <kis_assert.h>

#include "kis_abstract_input_action.h"
#include "kis_input_profile_manager.h"
#include "kis_shortcut_configuration.h"
#include "kis_zoom_and_rotate_action.h"

KisInputProfileMigrator::~KisInputProfileMigrator()
{
}

KisInputProfileMigratorFrom5::KisInputProfileMigratorFrom5(KisInputProfileManager *manager)
    : m_manager(manager)
{
    // FIXME(sh_zam): Should we declare this as "the default profile" somewhere?
    const QStringList profiles =
        KoResourcePaths::findAllAssets("data", "input/*.profile", KoResourcePaths::Recursive)
            .filter("kritadefault.profile");

    if (!profiles.empty()) {
        // This will be from the install location, so *has* to be the default, see KoResourcePaths for the
        // order in which it returns locations.
        m_defaultProfile = profiles.last();
    } else {
        qWarning() << "Default profile does not exist anywhere!";
    }
}

KisInputProfileMigratorFrom5::~KisInputProfileMigratorFrom5()
{
}

QList<KisShortcutConfiguration> KisInputProfileMigratorFrom5::defaultTouchShortcuts()
{
    QList<KisShortcutConfiguration> shortcuts = getShortcutsFromProfile(m_defaultProfile, m_manager);
    filterShortcuts(shortcuts, [](KisShortcutConfiguration shortcut) {
        return shortcut.type() == KisShortcutConfiguration::TouchGestureType ||
            shortcut.type() == KisShortcutConfiguration::NativeGestureType;
    });

    return shortcuts;
}

template <typename Func>
void KisInputProfileMigratorFrom5::filterShortcuts(QList<KisShortcutConfiguration> &shortcuts, Func pred)
{
    auto it = shortcuts.begin();
    while (it != shortcuts.end()) {
        KisShortcutConfiguration shortcut = *it;
        if (pred(shortcut)) {
            ++it;
        } else {
            it = shortcuts.erase(it);
        }
    }
}

QList<KisShortcutConfiguration> KisInputProfileMigrator::getShortcutsFromProfile(QString profile, KisInputProfileManager *manager)
{
    QList<KisShortcutConfiguration> shortcuts;

    KConfig config(profile, KConfig::SimpleConfig);

    const QList<KisAbstractInputAction *> actions = manager->actions();
    for (const auto action : actions) {
        if (!config.hasGroup(action->id())) {
            continue;
        }

        KConfigGroup group = config.group(action->id());
        for (const auto &groupEntry : group.entryMap()) {
            KisShortcutConfiguration shortcut;

            shortcut.setAction(action);
            if (shortcut.unserialize(groupEntry)) {
                shortcuts.append(shortcut);
            }
        }
    }
    return shortcuts;
}

QList<KisShortcutConfiguration> KisInputProfileMigratorFrom5::migrate(const ProfileEntry &profile)
{
    QList<KisShortcutConfiguration> shortcuts = getShortcutsFromProfile(profile.fullpath, m_manager);

    // we ignore the touch shortcuts, because they're from an older version
    filterShortcuts(shortcuts, [](KisShortcutConfiguration shortcut) {
        return shortcut.type() != KisShortcutConfiguration::TouchGestureType
            && shortcut.type() != KisShortcutConfiguration::NativeGestureType;
    });

    // now we add the default new shortcuts -- this should complete the migration.
    shortcuts.append(defaultTouchShortcuts());

    return shortcuts;
}


KisInputProfileMigratorFrom6::KisInputProfileMigratorFrom6(KisInputProfileManager *manager)
    : m_manager(manager)
{

}

KisInputProfileMigratorFrom6::~KisInputProfileMigratorFrom6()
{

}

QList<KisShortcutConfiguration> KisInputProfileMigratorFrom6::migrate(const ProfileEntry &profile)
{
    /**
     * Firstly, remove all native gesture shortcuts, since they change
     * semantics in Version 7
     */

    auto shortcuts = getShortcutsFromProfile(profile.fullpath, m_manager);
    for (auto it = shortcuts.begin(); it != shortcuts.end();) {
        if (it->type() == KisShortcutConfiguration::NativeGestureType) {
            it = shortcuts.erase(it);
        } else {
            ++it;
        }
    }

    /**
     * Now add the default connection between native gestures and
     * "Zoom and Rotate Canvas" action
     */

    auto actions = m_manager->actions();

    auto it = std::find_if(actions.begin(), actions.end(), kismpl::mem_equal_to(&KisAbstractInputAction::id, "Zoom and Rotate Canvas"));
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(it != actions.end(), shortcuts);

    KisShortcutConfiguration newNativeGestureShortcut;
    newNativeGestureShortcut.setAction(*it);
    newNativeGestureShortcut.setType(KisShortcutConfiguration::NativeGestureType);
    newNativeGestureShortcut.setNativeGesture(KisShortcutConfiguration::PinchGesture);
    newNativeGestureShortcut.setMode(KisZoomAndRotateAction::PanAndZoomAndRotateMode);
    shortcuts.append(newNativeGestureShortcut);

    return shortcuts;
}