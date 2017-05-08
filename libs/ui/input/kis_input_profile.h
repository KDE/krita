/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISINPUTPROFILE_H
#define KISINPUTPROFILE_H

#include <QObject>
#include <QMetaType>

class KisAbstractInputAction;
class KisShortcutConfiguration;
/**
 * \brief A container class for sets of shortcuts associated with an action.
 *
 *
 */
class KisInputProfile : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    KisInputProfile(QObject *parent = 0);
    /**
     * Destructor.
     */
    ~KisInputProfile() override;

    /**
     * \return The name of the profile.
     */
    QString name() const;

    /**
     * \return A list of all shortcuts available.
     */
    QList<KisShortcutConfiguration *> allShortcuts() const;
    /**
     * \return A list of shortcuts associated with the given action.
     *
     * \param action The action for which to list the shortcuts.
     */
    QList<KisShortcutConfiguration *> shortcutsForAction(KisAbstractInputAction *action) const;

    /**
     * Add a shortcut to this profile.
     *
     * \param shortcut The shortcut to add.
     */
    void addShortcut(KisShortcutConfiguration *shortcut);
    /**
     * Remove a shortcut from this profile.
     *
     * \param shortcut The shortcut to remove.
     */
    void removeShortcut(KisShortcutConfiguration *shortcut);

public Q_SLOTS:
    /**
     * Set the name of this profile.
     *
     * \param name The name to set.
     */
    void setName(const QString &name);

Q_SIGNALS:
    /**
     * Emitted when the name of this profile changes.
     */
    void nameChanged();

private:
    class Private;
    Private *const d;
};

#endif // KISINPUTPROFILE_H
