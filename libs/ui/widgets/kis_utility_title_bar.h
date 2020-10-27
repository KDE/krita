/*
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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
