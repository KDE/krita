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
#include "kis_show_palette_action.h"
#include "kis_tool_invocation_action.h"
#include "kis_zoom_action.h"
#include "kis_shortcut_configuration.h"
#include "kis_select_layer_action.h"
#include "kis_gamma_exposure_action.h"
#include "kis_change_frame_action.h"
#include "kis_zoom_and_rotate_action.h"
#include "KisCanvasOnlyAction.h"

#define PROFILE_VERSION 5


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
        emit currentProfileChanged();
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

    emit profilesChanged();

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
            emit currentProfileChanged();
        }

        emit profilesChanged();
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

        emit profilesChanged();


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

    emit profilesChanged();
}

QList< KisAbstractInputAction * > KisInputProfileManager::actions()
{
    return d->actions;
}


struct ProfileEntry {
    QString name;
    QString fullpath;
    int version;
};

void KisInputProfileManager::loadProfiles()
{
    //Remove any profiles that already exist
    d->currentProfile = nullptr;
    qDeleteAll(d->profiles);
    d->profiles.clear();

    //Look up all profiles (this includes those installed to $prefix as well as the user's local data dir)
    QStringList profiles = KoResourcePaths::findAllResources("data", "input/*.profile", KoResourcePaths::Recursive);

    dbgKrita << "profiles" << profiles;

    QMap<QString, QList<ProfileEntry> > profileEntries;

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
        if (entry.version != PROFILE_VERSION) {
            continue;
        }

        entry.name = config.group("General").readEntry("name");
        if (!profileEntries.contains(entry.name)) {
            profileEntries[entry.name] = QList<ProfileEntry>();
        }

        if (p.contains(".kde") || p.contains(".krita")) {
            // It's the user defined one, drop the others
            profileEntries[entry.name].clear();
            profileEntries[entry.name].append(entry);
            break;
        }
        else {
            profileEntries[entry.name].append(entry);
        }
    }

    QStringList profilePaths;

    Q_FOREACH(const QString & profileName, profileEntries.keys()) {

        if (profileEntries[profileName].isEmpty()) {
            continue;
        }

        // we have one or more entries for this profile name. We'll take the first,
        // because that's the most local one.
        ProfileEntry entry = profileEntries[profileName].first();

        QString path(QFileInfo(entry.fullpath).dir().absolutePath());
        if (!profilePaths.contains(path)) {
            profilePaths.append(path);
        }

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

//    QString profilePathsStr(profilePaths.join("' AND '"));
//    qDebug() << "input profiles were read from '" << qUtf8Printable(profilePathsStr) << "'.";

    KisConfig cfg(true);
    QString currentProfile = cfg.currentInputProfile();
    if (d->profiles.size() > 0) {
        if (currentProfile.isEmpty() || !d->profiles.contains(currentProfile)) {
            d->currentProfile = d->profiles.begin().value();
        }
        else {
            d->currentProfile = d->profiles.value(currentProfile);
        }
    }
    if (d->currentProfile) {
        emit currentProfileChanged();
    }
}

void KisInputProfileManager::saveProfiles()
{
    QString storagePath = KoResourcePaths::saveLocation("data", "input/", true);
    Q_FOREACH(KisInputProfile * p, d->profiles) {
        QString fileName = d->profileFileName(p->name());

        KConfig config(storagePath + fileName, KConfig::SimpleConfig);

        config.group("General").writeEntry("name", p->name());
        config.group("General").writeEntry("version", PROFILE_VERSION);

        Q_FOREACH(KisAbstractInputAction * action, d->actions) {
            KConfigGroup grp = config.group(action->id());
            grp.deleteGroup(); //Clear the group of any existing shortcuts.

            int index = 0;
            QList<KisShortcutConfiguration *> shortcuts = p->shortcutsForAction(action);
            Q_FOREACH(KisShortcutConfiguration * shortcut, shortcuts) {
                grp.writeEntry(QString("%1").arg(index++), shortcut->serialize());
            }
        }

        config.sync();
    }

    KisConfig config(false);
    config.setCurrentInputProfile(d->currentProfile->name());

    //Force a reload of the current profile in input manager and whatever else uses the profile.
    emit currentProfileChanged();
}

void KisInputProfileManager::resetAll()
{
    QString kdeHome = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QStringList profiles = KoResourcePaths::findAllResources("data", "input/*", KoResourcePaths::Recursive);

    Q_FOREACH (const QString &profile, profiles) {
        if(profile.contains(kdeHome)) {
            //This is a local file, remove it.
            QFile::remove(profile);
        }
    }

    //Load the profiles again, this should now only load those shipped with Krita.
    loadProfiles();

    emit profilesChanged();
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
    actions.append(new KisShowPaletteAction());
    actions.append(new KisSelectLayerAction());
    actions.append(new KisGammaExposureAction());
    actions.append(new KisChangeFrameAction());
    actions.append(new KisZoomAndRotateAction());
    actions.append(new KisCanvasOnlyAction());
}

QString KisInputProfileManager::Private::profileFileName(const QString &profileName)
{
    return profileName.toLower().remove(QRegExp("[^a-z0-9]")).append(".profile");
}
