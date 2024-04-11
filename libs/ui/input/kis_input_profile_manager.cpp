/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_profile_manager.h"
#include "kis_input_profile.h"

#include <QMap>
#include <QStringList>
#include <QDir>
#include <QGlobalStatic>

#include <KoResourcePaths.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include "kis_config.h"
#include "kis_alternate_invocation_action.h"
#include "kis_change_primary_setting_action.h"
#include "kis_pan_action.h"
#include "kis_rotate_canvas_action.h"
#include "KisPopupWidgetAction.h"
#include "kis_tool_invocation_action.h"
#include "kis_zoom_action.h"
#include "kis_shortcut_configuration.h"
#include "kis_select_layer_action.h"
#include "kis_gamma_exposure_action.h"
#include "kis_change_frame_action.h"
#include "kis_zoom_and_rotate_action.h"
#include "KisCanvasOnlyAction.h"
#include "KisTouchGestureAction.h"
#include "KisInputProfileMigrator.h"


class Q_DECL_HIDDEN KisInputProfileManager::Private
{
public:
    Private() : currentProfile(0) { }

    void createActions();
    QString profileFileName(const QString &profileName);

    KisInputProfile *currentProfile;

    QMap<QString, KisInputProfile *> profiles;

    QList<KisAbstractInputAction *> actions;
};

Q_GLOBAL_STATIC(KisInputProfileManager, inputProfileManager)

KisInputProfileManager *KisInputProfileManager::instance()
{
    return inputProfileManager;
}

QList< KisInputProfile * > KisInputProfileManager::profiles() const
{
    return d->profiles.values();
}

QStringList KisInputProfileManager::profileNames() const
{
    return d->profiles.keys();
}

KisInputProfile *KisInputProfileManager::profile(const QString &name) const
{
    if (d->profiles.contains(name)) {
        return d->profiles.value(name);
    }

    return 0;
}

KisInputProfile *KisInputProfileManager::currentProfile() const
{
    return d->currentProfile;
}

void KisInputProfileManager::setCurrentProfile(KisInputProfile *profile)
{
    if (profile && profile != d->currentProfile) {
        d->currentProfile = profile;
        Q_EMIT currentProfileChanged();
    }
}

KisInputProfile *KisInputProfileManager::addProfile(const QString &name)
{
    if (d->profiles.contains(name)) {
        return d->profiles.value(name);
    }

    KisInputProfile *profile = new KisInputProfile(this);
    profile->setName(name);
    d->profiles.insert(name, profile);

    Q_EMIT profilesChanged();

    return profile;
}

void KisInputProfileManager::removeProfile(const QString &name)
{
    if (d->profiles.contains(name)) {
        QString currentProfileName = d->currentProfile->name();

        delete d->profiles.value(name);
        d->profiles.remove(name);

        //Delete the settings file for the removed profile, if it exists
        QDir userDir(KoResourcePaths::saveLocation("data", "input/"));

        if (userDir.exists(d->profileFileName(name))) {
            userDir.remove(d->profileFileName(name));
        }

        if (currentProfileName == name) {
            d->currentProfile = d->profiles.begin().value();
            Q_EMIT currentProfileChanged();
        }

        Q_EMIT profilesChanged();
    }
}

bool KisInputProfileManager::renameProfile(const QString &oldName, const QString &newName)
{
    if (!d->profiles.contains(oldName)) {
        return false;
    }

    KisInputProfile *profile = d->profiles.value(oldName);
    if (profile) {
        d->profiles.remove(oldName);
        profile->setName(newName);
        d->profiles.insert(newName, profile);

        Q_EMIT profilesChanged();


        return true;
    }

    return false;
}

void KisInputProfileManager::duplicateProfile(const QString &name, const QString &newName)
{
    if (!d->profiles.contains(name) || d->profiles.contains(newName)) {
        return;
    }

    KisInputProfile *newProfile = new KisInputProfile(this);
    newProfile->setName(newName);
    d->profiles.insert(newName, newProfile);

    KisInputProfile *profile = d->profiles.value(name);
    QList<KisShortcutConfiguration *> shortcuts = profile->allShortcuts();
    Q_FOREACH(KisShortcutConfiguration * shortcut, shortcuts) {
        newProfile->addShortcut(new KisShortcutConfiguration(*shortcut));
    }

    Q_EMIT profilesChanged();
}

QList< KisAbstractInputAction * > KisInputProfileManager::actions()
{
    return d->actions;
}

void KisInputProfileManager::loadProfiles()
{
    //Remove any profiles that already exist
    d->currentProfile = nullptr;
    qDeleteAll(d->profiles);
    d->profiles.clear();

    //Look up all profiles (this includes those installed to $prefix as well as the user's local data dir)
    QStringList profiles = KoResourcePaths::findAllAssets("data", "input/*.profile", KoResourcePaths::Recursive);

    dbgKrita << "profiles" << profiles;

    // We don't use list here, because we're assuming we are only going to be changing the user directory and
    // there can only be one of a profile name.
    QMap<QString, ProfileEntry> profileEntriesToMigrate;
    QMap<QString, QList<ProfileEntry>> profileEntries;

    KisConfig cfg(true);

    // Get only valid entries...
    Q_FOREACH(const QString & p, profiles) {

        ProfileEntry entry;
        entry.fullpath = p;

        KConfig config(p, KConfig::SimpleConfig);
        if (!config.hasGroup("General") || !config.group("General").hasKey("name") || !config.group("General").hasKey("version")) {
            //Skip if we don't have the proper settings.
            continue;
        }

        // Only entries of exactly the right version can be considered
        entry.version = config.group("General").readEntry("version", 0);
        entry.name = config.group("General").readEntry("name");

        // NOTE: Migrating profiles doesn't just mean porting them to new version. Migrating a profile
        // may override the existing newer profile file.
        if (entry.version == PROFILE_VERSION - 1) {
            // we only utilize the first entry, because it is the most local one and the one which has to be
            // migrated.
            profileEntriesToMigrate[entry.name] = entry;

        } else if (entry.version == PROFILE_VERSION) {
            if (!profileEntries.contains(entry.name)) {
                profileEntries[entry.name] = QList<ProfileEntry>();
            }

            // let all the current version entries pile up in the list, it is only later where we check if it
            // is something we will use or a migrated entry.
            profileEntries[entry.name].append(entry);
        }
    }

    {
        const QString userLocalSaveLocation = KoResourcePaths::saveLocation("data", "input/");
        auto entriesIt = profileEntriesToMigrate.begin();
        while (entriesIt != profileEntriesToMigrate.end()) {
            ProfileEntry entry = *entriesIt;
            // if entry doesn't exist in profileEntries, means there is no corresponding new version of the
            // entry in user directory. Meaning, it is a certain candidate for migration.

            if (profileEntries.contains(entry.name)) {

                // we only need first() because if a user-local entry exists, it will be the first.
                ProfileEntry existingEntry = profileEntries[entry.name].first();

                // check if the entry's fullpath is a saveLocation, if so, we remove it from migration list.
                if (existingEntry.fullpath.startsWith(userLocalSaveLocation)) {
                    entriesIt = profileEntriesToMigrate.erase(entriesIt);
                } else {
                    // if the entry's fullpath is not a saveLocation, we will migrate it. Because (user's
                    // previous configuration + current default touch shortcuts) are better than. (All default
                    // shortcuts).
                    entriesIt++;

                    // Because this entry is supposed to be migrated, it will clash with an already existing
                    // default entry. So remove it.
                    profileEntries.remove(existingEntry.name);
                }
            } else {
                entriesIt++;
            }
        }
    }

    {
        KisInputProfileMigrator5To6 migrator(this);
        QMap<ProfileEntry, QList<KisShortcutConfiguration>> parsedProfilesToMigrate =
            migrator.migrate(profileEntriesToMigrate);

        for (ProfileEntry profileEntry : parsedProfilesToMigrate.keys()) {
            const QString storagePath = KoResourcePaths::saveLocation("data", "input/", true);

            {
                // the profile we have here uses the previous config, the only thing we need to make sure is
                // it doesn't overwrite the existing profile to preserve backwards compatibility.
                const QString profilePath = profileEntry.fullpath;
                QString oldProfileName = QFileInfo(profilePath).fileName();
                oldProfileName.replace(".profile", QString::number(PROFILE_VERSION - 1) + ".profile");

                QString oldProfilePath = storagePath + oldProfileName;
                // copy the profile to a new file but add version number to the name
                QFile::copy(profilePath, oldProfilePath);

                KConfig config(oldProfilePath, KConfig::SimpleConfig);
                config.group("General").writeEntry("migrated", PROFILE_VERSION);
            }

            KisInputProfile *newProfile = addProfile(profileEntry.name);
            QList<KisShortcutConfiguration> shortcuts = parsedProfilesToMigrate.value(profileEntry);
            for (const auto &shortcut : shortcuts) {
                newProfile->addShortcut(new KisShortcutConfiguration(shortcut));
            }

            // save the new profile with migrated shortcuts. We overwrite the previous version of file (which
            // previously has been moved for backward compatibility).
            saveProfile(newProfile, storagePath);
        }
    }

    Q_FOREACH(const QString & profileName, profileEntries.keys()) {

        if (profileEntries[profileName].isEmpty()) {
            continue;
        }

        // we have one or more entries for this profile name. We'll take the first,
        // because that's the most local one.
        ProfileEntry entry = profileEntries[profileName].first();

        KConfig config(entry.fullpath, KConfig::SimpleConfig);

        KisInputProfile *newProfile = addProfile(entry.name);
        Q_FOREACH(KisAbstractInputAction * action, d->actions) {
            if (!config.hasGroup(action->id())) {
                continue;
            }

            KConfigGroup grp = config.group(action->id());
            //Read the settings for the action and create the appropriate shortcuts.
            Q_FOREACH(const QString & entry, grp.entryMap()) {
                KisShortcutConfiguration *shortcut = new KisShortcutConfiguration;
                shortcut->setAction(action);

                if (shortcut->unserialize(entry)) {
                    newProfile->addShortcut(shortcut);
                }
                else {
                    delete shortcut;
                }
            }
        }
    }

    QString currentProfile = cfg.currentInputProfile();
    if (d->profiles.size() > 0) {
        if (currentProfile.isEmpty() || !d->profiles.contains(currentProfile)) {
            QString kritaDefault = QStringLiteral("Krita Default");
            if (d->profiles.contains(kritaDefault)) {
                d->currentProfile = d->profiles.value(kritaDefault);
            } else {
                d->currentProfile = d->profiles.begin().value();
            }
        }
        else {
            d->currentProfile = d->profiles.value(currentProfile);
        }
    }
    if (d->currentProfile) {
        Q_EMIT currentProfileChanged();
    }
}

void KisInputProfileManager::saveProfiles()
{
    QString storagePath = KoResourcePaths::saveLocation("data", "input/", true);
    Q_FOREACH(KisInputProfile * p, d->profiles) {
        saveProfile(p, storagePath);
    }

    KisConfig config(false);
    config.setCurrentInputProfile(d->currentProfile->name());

    //Force a reload of the current profile in input manager and whatever else uses the profile.
    Q_EMIT currentProfileChanged();
}

void KisInputProfileManager::saveProfile(KisInputProfile *profile, QString storagePath)
{
    const QString profilePath = storagePath + d->profileFileName(profile->name());
    KConfig config(profilePath, KConfig::SimpleConfig);

    config.group("General").writeEntry("name", profile->name());
    config.group("General").writeEntry("version", PROFILE_VERSION);

    Q_FOREACH(KisAbstractInputAction * action, d->actions) {
        KConfigGroup grp = config.group(action->id());
        grp.deleteGroup(); //Clear the group of any existing shortcuts.

        int index = 0;
        QList<KisShortcutConfiguration *> shortcuts = profile->shortcutsForAction(action);
        Q_FOREACH(KisShortcutConfiguration * shortcut, shortcuts) {
            grp.writeEntry(QString("%1").arg(index++), shortcut->serialize());
        }
    }

    config.sync();
}

QList<KisShortcutConfiguration *> KisInputProfileManager::getConflictingShortcuts(KisInputProfile *profile)
{
    QSet<KisShortcutConfiguration *> conflictedShortcuts;
    const QList<KisShortcutConfiguration *> shortcuts = profile->allShortcuts();
    for (auto startIt = shortcuts.constBegin(); startIt != shortcuts.constEnd(); ++startIt) {
        KisShortcutConfiguration *first = *startIt;
        for (auto index = startIt + 1; index != shortcuts.constEnd(); ++index) {
            KisShortcutConfiguration *second = *index;
            // since there can be multiple no-ops in the config and because no-ops are something a user can't
            // perform, there should be no conflicts.
            if (*first == *second && !first->isNoOp()) {
                conflictedShortcuts.insert(first);
                conflictedShortcuts.insert(second);
            }
        }
    }
    return conflictedShortcuts.values();
}

void KisInputProfileManager::resetAll()
{
    QString kdeHome = KoResourcePaths::getAppDataLocation();
    QStringList profiles = KoResourcePaths::findAllAssets("data", "input/*", KoResourcePaths::Recursive);

    Q_FOREACH (const QString &profile, profiles) {
        if(profile.contains(kdeHome)) {
            //This is a local file, remove it.
            QFile::remove(profile);
        }
    }

    //Load the profiles again, this should now only load those shipped with Krita.
    loadProfiles();

    Q_EMIT profilesChanged();
}

KisInputProfileManager::KisInputProfileManager(QObject *parent)
    : QObject(parent), d(new Private())
{
    d->createActions();
}

KisInputProfileManager::~KisInputProfileManager()
{
    qDeleteAll(d->profiles);
    qDeleteAll(d->actions);
    delete d;
}

void KisInputProfileManager::Private::createActions()
{
    //TODO: Make this plugin based
    //Note that the ordering here determines how things show up in the UI
    actions.append(new KisToolInvocationAction());
    actions.append(new KisAlternateInvocationAction());
    actions.append(new KisChangePrimarySettingAction());
    actions.append(new KisPanAction());
    actions.append(new KisRotateCanvasAction());
    actions.append(new KisZoomAction());
    actions.append(new KisPopupWidgetAction());
    actions.append(new KisSelectLayerAction());
    actions.append(new KisGammaExposureAction());
    actions.append(new KisChangeFrameAction());
    actions.append(new KisZoomAndRotateAction());
    actions.append(new KisTouchGestureAction());
}

QString KisInputProfileManager::Private::profileFileName(const QString &profileName)
{
    return profileName.toLower().remove(QRegExp("[^a-z0-9]")).append(".profile");
}
