/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
