/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TOOL_SELECT_MAGNETIC_OPTION_WIDGET_H
#define KIS_TOOL_SELECT_MAGNETIC_OPTION_WIDGET_H

#include <QWidget>

namespace Ui {
    class KisToolSelectMagneticOptionWidget;
}

class KisToolSelectMagneticOptionWidget : public QWidget {
    Q_OBJECT

public:
    enum SearchStartPoint {
        SearchFromLeft,
        SearchFromRight
    };

    // ColorLimitation must be the same order, as in the widget
    enum ColorLimitation {
        ColorLimitToAll=0,
        ColorLimitToRed=1,
        ColorLimitToGreen=2,
        ColorLimitToBlue=3
    };

    KisToolSelectMagneticOptionWidget(QWidget *parent = 0);
    ~KisToolSelectMagneticOptionWidget();

    Ui::KisToolSelectMagneticOptionWidget *ui;

protected:
    void changeEvent(QEvent *e);

};

#endif // KIS_TOOL_SELECT_MAGNETIC_OPTION_WIDGET_H
