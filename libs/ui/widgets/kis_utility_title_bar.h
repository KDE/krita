/*
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISUTILITYTITLEBAR_H
#define KISUTILITYTITLEBAR_H

#include <QWidget>

#ifdef Q_OS_MACOS
#include <sys/types.h>
#endif

#include "kritaui_export.h"

class QLabel;
class QHBoxLayout;
class QPushButton;

/** @brief A special utility titlebar with a title and controls,
 * as well as a central area for adding frequently used widgets.
 *
 * As a general design philosophy, we should try to reserve titlebar
 * widgets for things that are simple to use, frequently tweaked,
 * and core to the artists' workflow.
 */
class KRITAUI_EXPORT KisUtilityTitleBar : public QWidget
{
    Q_OBJECT

public:
    KisUtilityTitleBar(QWidget *parent = nullptr);
    KisUtilityTitleBar(QLabel *title, QWidget *parent = nullptr);

    virtual QSize sizeHint() const {return QSize(32,32);}

protected:
    QHBoxLayout *widgetAreaLayout;

    const int SPACING_UNIT = 16;
};

#endif // KISUTILITYTITLEBAR_H
