/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINPUTPROFILEMANAGER_H
#define KISINPUTPROFILEMANAGER_H

#include <QObject>

#include "kritaui_export.h"

class KisAbstractInputAction;
class KisInputProfile;

/**
 * \brief A class to manage a list of profiles and actions.
 *
 *
 */
class KRITAUI_EXPORT KisInputProfileManager : public QObject
{
    Q_OBJECT
public:
    KisInputProfileManager(QObject *parent = 0);
    ~KisInputProfileManager() override;
    Q_DISABLE_COPY(KisInputProfileManager)

    /**
     * Retrieve a profile by name.
     *
     * \param name The name of the profile to retrieve.
     *
     * \return The profile with the given name, or 0 if not found.
     */
    KisInputProfile *profile(const QString &name) const;
    /**
     * \return A list of all profiles.
     */
    QList<KisInputProfile *> profiles() const;
    /**
     * \return A list of the names of all profiles.
     */
    QStringList profileNames() const;

    /**
     * \return The current active profile.
     */
    KisInputProfile *currentProfile() const;
    /**
     * Set the current active profile.
     *
     * \param profile The profile to set as current.
     */
    void setCurrentProfile(KisInputProfile *profile);

    /**
     * Add a profile.
     *
     * \param name The name of the new profile.
     *
     * \return The new, empty profile or the non-empty profile if it already exists.
     */
    KisInputProfile *addProfile(const QString &name);
    /**
     * Remove a profile.
     *
     * This will remove the given profile from the list of profiles and delete it.
     *
     * \param name The profile to remove.
     */
    void removeProfile(const QString &name);
    /**
     * Rename a profile.
     *
     * \param oldName The current name of the profile.
     * \param newName The new name of the profile.
     *
     * \return true if successful, false if not.
     */
    bool renameProfile(const QString &oldName, const QString &newName);
    /**
     * Duplicate a profile.
     *
     * This creates a new profile with the given name and copies all
     * data from the old profile to the new profile.
     *
     * \param name The name of the profile to duplicate.
     * \param newName The name of the new profile.
     */
    void duplicateProfile(const QString &name, const QString &newName);

    /**
     * \return The list of all available actions.
     */
    QList< KisAbstractInputAction * > actions();

    /**
     * Load all profiles from the configuration stored on disk.
     */
    void loadProfiles();
    /**
     * Save all profiles to configuration on disk.
     */
    void saveProfiles();

    /**
     * Reset all profiles to the default state.
     *
     * This will remove all custom profiles the user created and reset any changed profiles.
     */
    void resetAll();

    /**
     * \return The singleton instance of this class.
     */
    static KisInputProfileManager *instance();

Q_SIGNALS:
    /**
     * Emitted when the list of profiles changes.
     */
    void profilesChanged();
    /**
     * Emitted when the current active profile changes.
     */
    void currentProfileChanged();

private:
    class Private;
    Private *const d;
};

#endif // KISINPUTPROFILEMANAGER_H
