/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>
   Copyright (C) 2006 Thiago Macieira <thiago@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KMAINWINDOWIFACE_P_H
#define KMAINWINDOWIFACE_P_H

#include <QDBusAbstractAdaptor>
#include <QMap>

class KXmlGuiWindow;

/**
 * @short D-Bus interface to KMainWindow.
 *
 * This is the main interface to the KMainWindow.  This will provide a consistent
 * D-Bus interface to all KDE applications that use it.
 *
 * @author Ian Reinhart Geiser <geiseri@yahoo.com>
 */
class KMainWindowInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KMainWindow")

public:
    /**
    Construct a new interface object.
    @param mainWindow - The parent KMainWindow object
    that will provide us with the KAction objects.
    */
    KMainWindowInterface(KXmlGuiWindow *mainWindow);
    /**
    Destructor
    Cleans up the dcop action proxy object.
    **/
    ~KMainWindowInterface() override;

public Q_SLOTS:
    /**
    Return a list of actions available to the application's window.
    @return A QStringList containing valid names actions.
    */
    QStringList actions();

    /**
    Activates the requested action.
    @param action The name of the action to activate.  The names of valid
    actions can be found by calling actions().
    @return The success of the operation.
    */
    bool activateAction(const QString &action);

    /**
    Disables the requested action.
    @param action The name of the action to disable.  The names of valid
    actions can be found by calling actions().
    @return The success of the operation.
    */
    bool disableAction(const QString &action);

    /**
    Enables the requested action.
    @param action The name of the action to enable.  The names of valid
    actions can be found by calling actions().
    @return The success of the operation.
    */
    bool enableAction(const QString &action);

    /**
    Returns the status of the requested action.
    @param action The name of the action.  The names of valid
    actions can be found by calling actions().
    @returns The state of the action, true - enabled, false - disabled.
    */
    bool actionIsEnabled(const QString &action);

    /**
    Returns the tool tip text of the requested action.
    @param action The name of the action to activate.  The names of valid
    actions can be found by calling actions().
    @return A QString containing the text of the action's tool tip.
    */
    QString actionToolTip(const QString &action);

    /**
    Returns the ID of the current main window.
    This is useful for automated screen captures or other evil
    widget fun.
    @return A integer value of the main window's ID.
    **/
    qlonglong winId();
    /**
    Copies a pixmap representation of the current main window to
    the clipboard.
    **/
    void grabWindowToClipBoard();
private:
    KXmlGuiWindow *m_MainWindow;
};

#endif // KMAINWINDOWIFACE_P_H

