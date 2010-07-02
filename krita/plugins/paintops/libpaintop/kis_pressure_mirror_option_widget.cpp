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

#include <klocale.h>

#include "kis_pressure_mirror_option.h"

KisPressureMirrorOptionWidget::KisPressureMirrorOptionWidget()
    : KisCurveOptionWidget(new KisPressureMirrorOption())
{
    QWidget* w = new QWidget;
    m_horizontalMirror = new QCheckBox(i18n("Horizontally"));
    m_horizontalMirror->setChecked(false);
    m_verticalMirror = new QCheckBox(i18n("Vertically"));
    m_verticalMirror->setChecked(false);
    
    connect(m_horizontalMirror, SIGNAL(toggled(bool)),SLOT(horizontalMirrorChanged(bool)));
    connect(m_verticalMirror, SIGNAL(toggled(bool)),SLOT(verticalMirrorChanged(bool)));
    
    QHBoxLayout* hl = new QHBoxLayout;
    hl->addWidget(m_horizontalMirror);
    hl->addWidget(m_verticalMirror);

    QVBoxLayout* vl = new QVBoxLayout;
    vl->addLayout(hl);
    vl->addWidget(curveWidget());

    w->setLayout(vl);
    setConfigurationPage(w);
    horizontalMirrorChanged(m_horizontalMirror->isChecked());
    verticalMirrorChanged(m_verticalMirror->isChecked());
}

void KisPressureMirrorOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    m_verticalMirror->setChecked(static_cast<KisPressureMirrorOption*>(curveOption())->isVerticalMirrorEnabled());
    m_horizontalMirror->setChecked(static_cast<KisPressureMirrorOption*>(curveOption())->isHorizontalMirrorEnabled());
}


void KisPressureMirrorOptionWidget::horizontalMirrorChanged(bool mirror)
{
    static_cast<KisPressureMirrorOption*>(curveOption())->enableHorizontalMirror(mirror);
    emit sigSettingChanged();
}

void KisPressureMirrorOptionWidget::verticalMirrorChanged(bool mirror)
{
    static_cast<KisPressureMirrorOption*>(curveOption())->enableVerticalMirror(mirror);
    emit sigSettingChanged();
}



