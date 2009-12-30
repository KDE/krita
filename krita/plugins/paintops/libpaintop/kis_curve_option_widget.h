/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) Sven Langkamp <sven.langkamp@gmail.com>, (C) 2009
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

#ifndef KIS_CURVE_OPTION_WIDGET_H
#define KIS_CURVE_OPTION_WIDGET_H

#include <kis_paintop_option.h>

class Ui_WdgCurveOption;
class KisCurveOption;
class KisDynamicSensor;

/**
 *
 * XXX; Add a reset button!
 */
class PAINTOP_EXPORT KisCurveOptionWidget : public KisPaintOpOption
{
    Q_OBJECT
public:    
    KisCurveOptionWidget(KisCurveOption* curveOption);
    ~KisCurveOptionWidget();

    virtual void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    virtual void readOptionSetting(const KisPropertiesConfiguration* setting);
    
    bool isCheckable();
    bool isChecked() const;
    void setChecked(bool checked);
    
protected:
    KisCurveOption* curveOption();
    
    QWidget* curveWidget();

private slots:

    void transferCurve();
    void setSensor(KisDynamicSensor* sensor);
    
private:
    QWidget* m_widget;
    Ui_WdgCurveOption* m_curveOptionWidget;
    KisCurveOption* m_curveOption;
};

#endif // KIS_CURVE_OPTION_WIDGET_H
