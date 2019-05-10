/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@kde.org>
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
#ifndef KISDIALOGSTATESAVER_H
#define KISDIALOGSTATESAVER_H

#include "kritawidgetutils_export.h"

#include <QWidget>
#include <QString>
#include <QMap>
#include <QVariant>

/**
 * @brief The KisDialogStateSaver class saves state for the specified
 * widget in the kritarc file and restores it. Simply call saveState
 * in your dialog's destructor, and use restoreState in the constructor.
 */
namespace KisDialogStateSaver
{
    /**
     * @brief saveState saves the state for the specified widgets
     * @param parent the parent at the top of the QObject hierarchy that contains the child widgets
     * @param dialogName the name for the section under which we will save the state
     * @return true if all the widgets could be saved, false if there was a problem
     */
    KRITAWIDGETUTILS_EXPORT void saveState(QWidget *parent, const QString &dialogName);

    /**
     * @brief restoreState restores the state of the dialog
     * @param parent the parent at the top of the QObject hierarchy that contains the child widgets
     * @param dialogName the name for the section under which we will restore the state
     * @param defaults: contains default values for widgets. If there are widgets for which no default
     *  has been specified, the default value created by QVariant will be used.
     * in the variant part of the map.
     * @return true if all the widgets could be restored, false if there was a problem
     */
    KRITAWIDGETUTILS_EXPORT void restoreState(QWidget *parent, const QString &dialogName, const QMap<QString, QVariant> &defaults = QMap<QString, QVariant>());
};

#endif // KISDIALOGSTATESAVER_H
