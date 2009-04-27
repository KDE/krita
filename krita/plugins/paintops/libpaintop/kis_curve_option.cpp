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
#include "kis_curve_option.h"
#include "widgets/kis_curve_widget.h"

KisCurveOption::KisCurveOption(const QString & label, const QString& name, bool checked)
    : KisPaintOpOption(label, checked)
    , m_customCurve(false)
    , m_curveWidget( new KisCurveWidget() )
    , m_name( name )
{
    setConfigurationPage(m_curveWidget);
    m_curve = QVector<double>(256, 0.0);
    connect(m_curveWidget, SIGNAL(modified()), this, SLOT(transferCurve()));
}

void KisCurveOption::transferCurve()
{
    double value;
    for (int i = 0; i < 256; i++) {
        value = m_curveWidget->getCurveValue(i / 255.0);
        if (value < PRESSURE_MIN)
            m_curve[i] = PRESSURE_MIN;
        else if (value > PRESSURE_MAX)
            m_curve[i] = PRESSURE_MAX;
        else
            m_curve[i] = value;
    }
    m_customCurve = true;

    emit sigSettingChanged();
}


void KisCurveOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    if ( m_checkable ) {
        setting->setProperty( "Pressure" + m_name, isChecked() );
    }
    setting->setProperty( "Custom" + m_name, m_customCurve );
    if ( m_customCurve ) {
        for (int i = 0; i < 256; i++) {
            setting->setProperty( QString(m_name + "Curve%1").arg(i), m_curve[i]);
        }
    }
}

void KisCurveOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    if ( m_checkable ) {
        setChecked( setting->getBool( "Pressure" + m_name, false ) );
    }
    m_customCurve = setting->getBool( "Custom" + m_name, false );

    for (int i = 0; i < 256; i++) {
        if (m_customCurve) {
                m_curve[i] = setting->getDouble(QString(m_name + "Curve%0").arg(i), i / 255.0);
        }
    }
    emit sigSettingChanged();
}
