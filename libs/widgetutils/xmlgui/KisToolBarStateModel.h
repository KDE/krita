/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTOOLBARSTATEMODEL_H
#define KISTOOLBARSTATEMODEL_H

#include <kritawidgetutils_export.h>

#include <QObject>

#include <lager/state.hpp>
#include <lager/extra/qt.hpp>


class KRITAWIDGETUTILS_EXPORT KisToolBarStateModel : public QObject
{
    Q_OBJECT
public:
    KisToolBarStateModel();

private:
    lager::state<bool, lager::automatic_tag> m_toolBarsLocked;

public:
    LAGER_QT_CURSOR(bool, toolBarsLocked);
};

#endif // KISTOOLBARSTATEMODEL_H
