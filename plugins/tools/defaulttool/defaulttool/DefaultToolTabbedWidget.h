/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef DEFAULTTOOLTABBEDWIDGET_H
#define DEFAULTTOOLTABBEDWIDGET_H

#include <KoTitledTabWidget.h>

class KoInteractionTool;

class DefaultToolTabbedWidget : public KoTitledTabWidget
{
    Q_OBJECT

public:
    explicit DefaultToolTabbedWidget(KoInteractionTool *tool, QWidget *parent = 0);
    ~DefaultToolTabbedWidget();

    enum TabType {
        GeometryTab,
        StrokeTab,
        FillTab
    };

Q_SIGNALS:
    void sigSwitchModeEditFillGradient(bool value);

private Q_SLOTS:
    void slotCurrentIndexChanged(int current);

private:
    int m_oldTabIndex;
};

#endif // DEFAULTTOOLTABBEDWIDGET_H
