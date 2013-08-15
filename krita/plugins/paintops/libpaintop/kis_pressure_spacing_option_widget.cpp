/*
 * Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>


#include <klocale.h>

#include <kis_slider_spin_box.h>

#include "kis_curve_option_widget.h"
#include "kis_pressure_spacing_option.h"
#include "kis_pressure_spacing_option_widget.h"

KisPressureSpacingOptionWidget::KisPressureSpacingOptionWidget():
    KisCurveOptionWidget(new KisPressureSpacingOption())
{
    QCheckBox *isotropicSpacing = new QCheckBox(i18n("Isotropic Spacing"));

    QVBoxLayout* vl = new QVBoxLayout;
    vl->setMargin(0);
    vl->addWidget(isotropicSpacing);
    vl->addWidget(KisCurveOptionWidget::curveWidget());

    QWidget* w = new QWidget;
    w->setLayout(vl);

    KisCurveOptionWidget::setConfigurationPage(w);


    connect(isotropicSpacing, SIGNAL(stateChanged(int)),
            this, SLOT(setIsotropicSpacing(int)));

    setIsotropicSpacing(false);
}

void KisPressureSpacingOptionWidget::setIsotropicSpacing(int isotropic)
{
    dynamic_cast<KisPressureSpacingOption*>(KisCurveOptionWidget::curveOption())->setIsotropicSpacing(isotropic);
    emit sigSettingChanged();
}
