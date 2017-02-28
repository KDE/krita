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
    explicit Action(QObject *parent = 0);
    Action(const QString &name, QAction *action, QObject *parent = 0);
    virtual ~Action();

public Q_SLOTS:

    QString name() const;
    void setName(QString name);

    bool isCheckable() const;
    void setCheckable(bool value);

    bool isChecked() const;
    void setChecked(bool value);

    QString shortcut() const;
    void setShortcut(QString value);

    bool isVisible() const;
    /**
     * @brief setVisible determines whether the action will be visible in the scripting menu.
     * @param value true if the action is to be shown in the menu, false otherwise
     */
    void setVisible(bool value);

    bool isEnabled() const;
    void setEnabled(bool value);

    void setToolTip(QString tooltip);

    void trigger();

Q_SIGNALS:

    void triggered(bool);

private:
    struct Private;
    Private *const d;

};


#endif // LIBKIS_ACTION_H
