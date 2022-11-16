/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISINPUTPROFILEMIGRATOR_H_
#define __KISINPUTPROFILEMIGRATOR_H_

#include <QList>
#include <QMap>

class KisShortcutConfiguration;
class KisInputProfileManager;
class KisAbstractInputAction;

struct ProfileEntry
{
    QString name;
    QString fullpath;
    int version;

    int operator<(const ProfileEntry other) const
    {
        return this->name < other.name;
    }
};

class KisInputProfileMigrator
{
public:
    virtual ~KisInputProfileMigrator();

    virtual QMap<ProfileEntry, QList<KisShortcutConfiguration>>
    migrate(const QMap<QString, ProfileEntry> profiles) = 0;
};

/**
 * Migrates Krita profile version 5 to 6
 */
class KisInputProfileMigrator5To6 : public KisInputProfileMigrator
{

public:
    KisInputProfileMigrator5To6(KisInputProfileManager *manager);
    ~KisInputProfileMigrator5To6() override;

    QMap<ProfileEntry, QList<KisShortcutConfiguration>>
    migrate(const QMap<QString, ProfileEntry> profiles) override;

private:
    QList<KisShortcutConfiguration> defaultTouchShortcuts();

    template <typename Func>
    void filterShortcuts(QList<KisShortcutConfiguration> &shortcuts, Func func);

    QList<KisShortcutConfiguration> getShortcutsFromProfile(QString profile) const;

private:
    KisInputProfileManager *m_manager;
    QStringList m_profilesList;
    QString m_defaultProfile;
};

#endif // __KISINPUTPROFILEMIGRATOR_H_
