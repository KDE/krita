/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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

#ifndef KIS_CURVE_OPTION_H
#define KIS_CURVE_OPTION_H

#include <QObject>
#include <QVector>

#include "kis_global.h"
#include "kis_paintop_option.h"
#include "kis_paint_information.h"
#include "krita_export.h"
#include "kis_dynamic_sensor.h"

class KisCurveWidget;
class Ui_WdgCurveOption;
class KisDynamicSensor;
/**
 * KisCurveOption is the base class for paintop options that are
 * defined through a curve.
 *
 * XXX; Add a reset button!
 */
class PAINTOP_EXPORT KisCurveOption : public KisPaintOpOption
{

    Q_OBJECT

public:

    KisCurveOption(const QString & label, const QString& name, bool checked = true);
    ~KisCurveOption();
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;

    void readOptionSetting(const KisPropertiesConfiguration* setting);

    KisDynamicSensor* sensor();
protected:

    double computeValue(const KisPaintInformation& info) const {
        double v = m_sensor->parameter(info);
        if (customCurve()) {
            return scaleToCurve(v);
        } else {
            return v;
        }
    }

    double scaleToCurve(double pressure) const {
        int offset = int(255.0 * pressure);
        if (offset < 0)
            offset = 0;
        if (offset > 255)
            offset =  255; // Was: clamp(..., 0, 255);
        return m_curve[offset];
    }

    bool customCurve() const {
        return m_customCurve;
    }

private slots:

    void transferCurve();
    void setSensor(KisDynamicSensor* sensor);

protected:

    KisDynamicSensor* m_sensor;
    bool m_customCurve;
    QWidget* m_widget;
    Ui_WdgCurveOption* m_curveOption;
    QVector<double> m_curve;
    QString m_name;
};

#endif
