/*
 *  SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
     * @param defaults: contains default values for widgets. This overrides what is stored in the config
     * file. If there is no value in the config file, and no default specified, the value set for
     * the widget (for instance in the ui file) will be used.
     * @return true if all the widgets could be restored, false if there was a problem
     */
    KRITAWIDGETUTILS_EXPORT void restoreState(QWidget *parent, const QString &dialogName, const QMap<QString, QVariant> &defaults = QMap<QString, QVariant>());
};

#endif // KISDIALOGSTATESAVER_H
