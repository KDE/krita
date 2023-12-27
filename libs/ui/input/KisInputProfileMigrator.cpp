/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisInputProfileMigrator.h"

#include <KConfig>
#include <KConfigGroup>
#include <KoResourcePaths.h>

#include <QDebug>

#include "kis_abstract_input_action.h"
#include "kis_input_profile_manager.h"
#include "kis_shortcut_configuration.h"

KisInputProfileMigrator::~KisInputProfileMigrator()
{
}

KisInputProfileMigrator5To6::KisInputProfileMigrator5To6(KisInputProfileManager *manager)
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

KisInputProfileMigrator5To6::~KisInputProfileMigrator5To6()
{
}

QList<KisShortcutConfiguration> KisInputProfileMigrator5To6::defaultTouchShortcuts()
{
    QList<KisShortcutConfiguration> shortcuts = getShortcutsFromProfile(m_defaultProfile);
    filterShortcuts(shortcuts, [](KisShortcutConfiguration shortcut) {
        return shortcut.type() == KisShortcutConfiguration::GestureType;
    });

    return shortcuts;
}

template <typename Func>
void KisInputProfileMigrator5To6::filterShortcuts(QList<KisShortcutConfiguration> &shortcuts, Func pred)
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

QList<KisShortcutConfiguration> KisInputProfileMigrator5To6::getShortcutsFromProfile(QString profile) const
{
    QList<KisShortcutConfiguration> shortcuts;

    KConfig config(profile, KConfig::SimpleConfig);

    const QList<KisAbstractInputAction *> actions = m_manager->actions();
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

QMap<ProfileEntry, QList<KisShortcutConfiguration>>
KisInputProfileMigrator5To6::migrate(const QMap<QString, ProfileEntry> profiles)
{
    QMap<ProfileEntry, QList<KisShortcutConfiguration>> parsedProfiles;
    for (const auto &profile : profiles) {
        QList<KisShortcutConfiguration> shortcuts = getShortcutsFromProfile(profile.fullpath);

        // we ignore the touch shortcuts, because they're from an older version
        filterShortcuts(shortcuts, [](KisShortcutConfiguration shortcut) {
            return shortcut.type() != KisShortcutConfiguration::GestureType;
        });

        // now we add the default new shortcuts -- this should complete the migration.
        shortcuts.append(defaultTouchShortcuts());

        parsedProfiles[profile] = shortcuts;
    }

    return parsedProfiles;
}
