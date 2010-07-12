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

#ifndef KIS_COLOR_SELECTOR_TYPE_WIDGET_H
#define KIS_COLOR_SELECTOR_TYPE_WIDGET_H

class KisCanvas2;
class KisColorSelectorTypeWidgetPrivate;

#include <QComboBox>

#include "kis_color_selector.h"

class KisColorSelectorTypeWidget : public QComboBox
{
    Q_OBJECT
public:
    KisColorSelectorTypeWidget(QWidget* parent=0);
    ~KisColorSelectorTypeWidget();
    void hidePopup();
    void showPopup();
    void setCanvas(KisCanvas2* canvas);
    KisColorSelector::Configuration configuration() const;
protected:
    void paintEvent(QPaintEvent *e);
public slots:
    void setConfiguration(KisColorSelector::Configuration);
private:
    KisColorSelectorTypeWidgetPrivate* m_private;
    KisColorSelector::Configuration m_configuration;
    KisColorSelector m_currentSelector;
};

#endif // KIS_COLOR_SELECTOR_TYPE_WIDGET_H
