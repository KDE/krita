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
class KoColorSpace;
class QBoxLayout;
class KisCanvas2;
class KAction;

class KisColorSelectorContainer : public QWidget
{
Q_OBJECT
public:
    explicit KisColorSelectorContainer(QWidget *parent = 0);
    void setCanvas(KisCanvas2* canvas);
    void unsetCanvas();

    enum ShadeSelectorType{MyPaintSelector, MinimalSelector, NoSelector};

signals:
    void openSettings();
    void settingsChanged();

protected slots:
    void updateSettings();
    void reactOnLayerChange();

protected:
    void resizeEvent(QResizeEvent *);

private:
    KisColorSelector* m_colorSelector;
    KisMyPaintShadeSelector* m_myPaintShadeSelector;
    KisMinimalShadeSelector* m_minimalShadeSelector;
    QWidget* m_shadeSelector;

    bool m_shadeSelectorHideable;
    bool m_allowHorizontalLayout;

    QBoxLayout* m_widgetLayout;

    KAction* m_colorSelAction;
    KAction* m_mypaintAction;
    KAction* m_minimalAction;

    KisCanvas2* m_canvas;
};

#endif // KIS_COLOR_SELECTOR_CONTAINER_H
