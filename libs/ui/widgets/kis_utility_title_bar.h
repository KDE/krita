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

class KRITAUI_EXPORT KisUtilityTitleBar : public QWidget
{
    Q_OBJECT

public:
    KisUtilityTitleBar(QWidget *parent = nullptr);
    KisUtilityTitleBar(QLabel *title, QWidget *parent = nullptr);

    virtual QSize sizeHint() const {return QSize(32,32);}

protected:
    QLabel *title;
    QHBoxLayout *widgetArea;

    const int SPACING_UNIT = 16;

private:
    QPushButton *floatButton;
    QPushButton *closeButton;
};

#endif // KISUTILITYTITLEBAR_H
