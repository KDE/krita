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

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * Action
 */
class KRITALIBKIS_EXPORT Action : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Action)

    Q_PROPERTY(QString Name READ name WRITE setName)
    Q_PROPERTY(QString Menu READ menu WRITE setMenu)
    Q_PROPERTY(bool Checkable READ checkable WRITE setCheckable)
    Q_PROPERTY(bool Checked READ checked WRITE setChecked)
    Q_PROPERTY(QString Shortcut READ shortcut WRITE setShortcut)
    Q_PROPERTY(bool Visible READ visible WRITE setVisible)
    Q_PROPERTY(bool Enabled READ enabled WRITE setEnabled)

public:
    explicit Action(QObject *parent = 0);
    virtual ~Action();

    QString name() const;
    void setName(QString value);

    QString menu() const;
    void setMenu(QString value);

    bool checkable() const;
    void setCheckable(bool value);

    bool checked() const;
    void setChecked(bool value);

    QString shortcut() const;
    void setShortcut(QString value);

    bool visible() const;
    void setVisible(bool value);

    bool enabled() const;
    void setEnabled(bool value);



public Q_SLOTS:

    void Trigger();

    void Toggle(bool toggle);



Q_SIGNALS:

    void Toggled(bool toggle);
    void Triggered();


private:
    struct Private;
    const Private *const d;

};

#endif // LIBKIS_ACTION_H
