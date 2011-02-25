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
 */
class PAINTOP_EXPORT KisCurveOption
{
public:

    KisCurveOption(const QString & label, const QString& name, const QString& category, bool checked);
    virtual ~KisCurveOption();
    virtual void writeOptionSetting(KisPropertiesConfiguration* setting) const;

    virtual void readOptionSetting(const KisPropertiesConfiguration* setting);

    const QString & label() const;
    const QString& category() const;

    KisCubicCurve curve() const;
    void setCurve(const KisCubicCurve& curve);

    KisDynamicSensor* sensor() const;
    void setSensor(KisDynamicSensor* sensor);

    bool isCheckable();

    bool isChecked() const;
    void setChecked(bool checked);
    const KisCurveLabel& minimumLabel() const;
    const KisCurveLabel& maximumLabel() const;
    
protected:
    void setMinimumLabel(const KisCurveLabel& _label);
    void setMaximumLabel(const KisCurveLabel& _label);
    /**
     * Read the option using the prefix in argument
     */
    void readNamedOptionSetting(const QString& prefix, const KisPropertiesConfiguration* setting);

protected:

    double computeValue(const KisPaintInformation& info) const {
        return m_sensor->parameter(info);
    }


protected:
    QString m_label, m_category;
    KisDynamicSensor* m_sensor;
    QString m_name;
    bool m_checkable;
    bool m_checked;
private:
    KisCurveLabel m_minimumLabel, m_maximumLabel;
};

#endif
