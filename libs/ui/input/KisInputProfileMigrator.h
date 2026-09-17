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

    virtual QList<KisShortcutConfiguration> migrate(const ProfileEntry &profile) = 0;

protected:
    static QList<KisShortcutConfiguration> getShortcutsFromProfile(QString profile, KisInputProfileManager *manager);
};

/**
 * Migrates Krita profile from version 5 to the current version
 */
class KisInputProfileMigratorFrom5 : public KisInputProfileMigrator
{

public:
    KisInputProfileMigratorFrom5(KisInputProfileManager *manager);
    ~KisInputProfileMigratorFrom5() override;

    QList<KisShortcutConfiguration> migrate(const ProfileEntry &profile) override;

private:
    QList<KisShortcutConfiguration> defaultTouchShortcuts();

    template <typename Func>
    void filterShortcuts(QList<KisShortcutConfiguration> &shortcuts, Func func);

private:
    KisInputProfileManager *m_manager;
    QStringList m_profilesList;
    QString m_defaultProfile;
};

/**
 * Migrates Krita profile from version 6 to the current version
 */
class KisInputProfileMigratorFrom6 : public KisInputProfileMigrator
{

public:
    KisInputProfileMigratorFrom6(KisInputProfileManager *manager);
    ~KisInputProfileMigratorFrom6() override;

    QList<KisShortcutConfiguration> migrate(const ProfileEntry &profile) override;

private:
    KisInputProfileManager *m_manager;
};

#endif // __KISINPUTPROFILEMIGRATOR_H_
