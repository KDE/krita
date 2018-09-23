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

#include "kis_pressure_mirror_option_widget.h"

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include <klocalizedstring.h>

#include "kis_pressure_mirror_option.h"

KisPressureMirrorOptionWidget::KisPressureMirrorOptionWidget()
    : KisCurveOptionWidget(new KisPressureMirrorOption(), i18n("Not mirrored"), i18n("Mirrored"))
{
    setObjectName("KisPressureMirrorOptionWidget");

    QWidget* w = new QWidget;
    m_horizontalMirror = new QCheckBox(i18n("Horizontal"));
    m_horizontalMirror->setChecked(false);
    m_verticalMirror = new QCheckBox(i18n("Vertical"));
    m_verticalMirror->setChecked(false);

    connect(m_horizontalMirror, SIGNAL(toggled(bool)), SLOT(horizontalMirrorChanged(bool)));
    connect(m_verticalMirror, SIGNAL(toggled(bool)), SLOT(verticalMirrorChanged(bool)));

    QLabel* directionLabel = new QLabel(i18n("Direction: "));

    QHBoxLayout* hl = new QHBoxLayout;
    hl->setContentsMargins(9,9,9,0); // no bottom spacing

    hl->addWidget(directionLabel);
    hl->addWidget(m_horizontalMirror);
    hl->addWidget(m_verticalMirror);
    hl->addStretch(1);



    QVBoxLayout* vl = new QVBoxLayout;
    vl->setMargin(0);
    vl->addLayout(hl);
    vl->addWidget(curveWidget());

    w->setLayout(vl);
    setConfigurationPage(w);
    horizontalMirrorChanged(m_horizontalMirror->isChecked());
    verticalMirrorChanged(m_verticalMirror->isChecked());
}

void KisPressureMirrorOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    m_verticalMirror->setChecked(static_cast<KisPressureMirrorOption*>(curveOption())->isVerticalMirrorEnabled());
    m_horizontalMirror->setChecked(static_cast<KisPressureMirrorOption*>(curveOption())->isHorizontalMirrorEnabled());
}


void KisPressureMirrorOptionWidget::horizontalMirrorChanged(bool mirror)
{
    static_cast<KisPressureMirrorOption*>(curveOption())->enableHorizontalMirror(mirror);
    emitSettingChanged();
}

void KisPressureMirrorOptionWidget::verticalMirrorChanged(bool mirror)
{
    static_cast<KisPressureMirrorOption*>(curveOption())->enableVerticalMirror(mirror);
    emitSettingChanged();
}



