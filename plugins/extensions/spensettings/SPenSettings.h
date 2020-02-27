/*
 *  Copyright (c) 2020 Anna Medonosová <anna.medonosova@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SPENSETTINGS_H
#define SPENSETTINGS_H

#include <QObject>
#include <QMap>
#include <QScopedPointer>
#include <KisActionPlugin.h>

class KisAction;

class SPenSettings : public KisActionPlugin
{
    Q_OBJECT
public:
    SPenSettings(QObject* parent, const QVariantList&);
    ~SPenSettings();

    enum Action {
        Click,
        DoubleClick,
        SwipeUp,
        SwipeDown,
        SwipeLeft,
        SwipeRight,
        CircleCW,
        CircleCCW
    };


public Q_SLOTS:
    void slotActivateAction(SPenSettings::Action gestureType);
    void slotLoadSettings();
    void slotTriggerPopupPalette();

private:
    QMap<SPenSettings::Action, QString> m_actionMap;
    QScopedPointer<KisAction> m_actionShowPopupPalette;
};

#endif // SPENSETTINGS_H
