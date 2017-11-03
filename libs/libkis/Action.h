/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_ACTION_H
#define LIBKIS_ACTION_H

#include <kis_action.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * Action encapsulates a KisAction. By default, actions are put in the Tools/Scripts menu.
 */
class KRITALIBKIS_EXPORT Action : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Action Create a new action object
     * @param parent the parent if it's in a QObject hierarchy
     */
    explicit Action(QObject *parent = 0);

    /**
     * @brief Action Create a new action object
     * @param name the name of the action
     * @param action the QAction it wraps
     * @param parent the parent if it's in a QObject hierarchy
     */
    Action(const QString &name, QAction *action, QObject *parent = 0);
    ~Action() override;

    bool operator==(const Action &other) const;
    bool operator!=(const Action &other) const;

public Q_SLOTS:

    /**
     * @return the user-visible text of the action.
     */
    QString text() const;

    /**
     * set the user-visible text of the action to @param text.
     */
    void setText(QString text);

    /**
     * @return the internal name of the action.
     */
    QString name() const;

    /**
     * set the name of the action to @param name. This is not the user-visible name, but the internal one
     */
    void setName(QString name);

    /**
     * @return true if the action is checkable, false if it isn't*
     */
    bool isCheckable() const;

    /**
     * Set the action action checkable if @param value is true, unchecked if it's false
     */
    void setCheckable(bool value);

    /**
     * @return true if the action is checked, false if it isn't
     */
    bool isChecked() const;

    /**
     * Set the action checked if @param value is true, unchecked if it's false
     */
    void setChecked(bool value);

    /**
     * Return the action's shortcut as a string
     */
    QString shortcut() const;

    /**
     * set the action's shortcut to the given string.
     * @code
     * action.setShortcut("CTRL+SHIFT+S")
     * @endcode
     */
    void setShortcut(QString value);

    bool isVisible() const;
    /**
     * @brief setVisible determines whether the action will be visible in the scripting menu.
     * @param value true if the action is to be shown in the menu, false otherwise
     */
    void setVisible(bool value);

    /**
     * @return true if the action is enabled, false if not
     */
    bool isEnabled() const;

    /**
     * Set the action enabled or disabled according to @param value
     */
    void setEnabled(bool value);

    /**
     * Set the tooltip to the given @param tooltip
     */
    void setToolTip(QString tooltip);

    /**
     * @return the tooltip text
     */
    QString tooltip() const;

    /**
     * Trigger this action
     */
    void trigger();

    /**
     * @brief setMenu determines in which menu the action will be placed. The default is tools/scripts
     * @param menu the menu where the action should go, / -separated to drill down the hierarchy
     */
    void setMenu(const QString menu);

    /**
     * @return the menu in which this action is to be placed.
     */
    QString menu() const;


Q_SIGNALS:

    /**
     * Emitted whenever the action is triggered.
     */
    void triggered(bool);

private:
    struct Private;
    Private *const d;

};


#endif // LIBKIS_ACTION_H
