/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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
 */

#ifndef KIS_COLOR_SELECTOR_CONTAINER_H
#define KIS_COLOR_SELECTOR_CONTAINER_H

#include <QWidget>

class KisColorSelector;
class KisMyPaintShadeSelector;
class KisMinimalShadeSelector;
class QBoxLayout;

class KisColorSelectorContainer : public QWidget
{
Q_OBJECT
public:
    explicit KisColorSelectorContainer(QWidget *parent = 0);

    enum ShadeSelectorType{MyPaintSelector, MinimalSelector, NoSelector};

    void setShadeSelectorType(int type);
    void setShadeSelectorHideable(bool hideable);
    void setAllowHorizontalLayout(bool allow);

signals:
    void openSettings();

protected:
    void resizeEvent(QResizeEvent *);

private:
    KisColorSelector* m_colorSelector;
    KisMyPaintShadeSelector* m_myPaintShadeSelector;
    KisMinimalShadeSelector* m_minimalShadeSelector;
    QWidget* m_shadeSelector;

    bool m_shadeSelectorHideable;
    bool m_allowHorizontalLayout;

    QBoxLayout* m_buttonLayout;
    QBoxLayout* m_widgetLayout;
};

#endif // KIS_COLOR_SELECTOR_CONTAINER_H
