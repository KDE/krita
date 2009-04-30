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

#include "kis_pressure_rate_option.h"

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klocale.h>

#include <kis_paint_device.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>
#include <KoColorSpace.h>

KisPressureRateOption::KisPressureRateOption()
    : KisCurveOption(i18n("Rate"), "Rate")
{
    QWidget* w = new QWidget;
    QLabel* rateLabel = new QLabel(i18n("Rate: "));
    m_rateSlider = new QSlider();
    m_rateSlider->setMinimum(0);
    m_rateSlider->setMaximum(100);
    m_rateSlider->setPageStep(1);
    m_rateSlider->setValue(50);
    m_rateSlider->setOrientation(Qt::Horizontal);
    QHBoxLayout* hl = new QHBoxLayout;
    hl->addWidget( rateLabel );
    hl->addWidget( m_rateSlider );

    QVBoxLayout* vl = new QVBoxLayout;
    vl->addLayout( hl );
    vl->addWidget( m_curveWidget );

    w->setLayout( vl );
    setConfigurationPage( w );
}

int KisPressureRateOption::rate() const
{
    return (m_rateSlider->value() * 255) / 100;
}

void KisPressureRateOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting( setting );
    setting->setProperty( "PressureRate", rate() );
}

void KisPressureRateOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting( setting );
    m_rateSlider->setValue( setting->getInt( "PressureRate" ) );
}



quint8 KisPressureRateOption::apply( quint8 opacity, qint32 sw,  qint32 sh, KisPaintDeviceSP srcdev, double pressure) const
{
    opacity = rate();

    if (isChecked()) {
        if (customCurve()) {
            opacity = qBound((qint32)OPACITY_TRANSPARENT, 
                             (qint32)(double(opacity) * scaleToCurve(pressure) / PRESSURE_DEFAULT),
                             (qint32)OPACITY_OPAQUE);

        } else {
            opacity = qBound((qint32)OPACITY_TRANSPARENT,
                             (qint32)(double(opacity) * pressure / PRESSURE_DEFAULT),
                             (qint32)OPACITY_OPAQUE);
        }
    }

#if 0
    // TODO It's also applied in the smudgeop, do other paintops that require a rate
    // needs to do this to their srcDev ?
    KisRectIterator it = srcdev->createRectIterator(0, 0, sw, sh);
    KoColorSpace* cs = srcdev->colorSpace();

    while (not it.isDone()) {
        cs->setAlpha(it.rawData(), (cs->alpha(it.rawData()) * opacity) / OPACITY_OPAQUE, 1);
        ++it;
    }
    return OPACITY_OPAQUE - opacity;
#else
    return opacity;
#endif


}

