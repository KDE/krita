/*
 *  SPDX-FileCopyrightText: 2020 Anna Medonosová <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
