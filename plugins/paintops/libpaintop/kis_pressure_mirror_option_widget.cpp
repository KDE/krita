/*
 * SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_pressure_mirror_option_widget.h"

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klocalizedstring.h>

#include "kis_pressure_mirror_option.h"

KisPressureMirrorOptionWidget::KisPressureMirrorOptionWidget()
    : KisCurveOptionWidget(new KisPressureMirrorOption(), i18n("Not mirrored"), i18n("Mirrored"))
{
    setObjectName("KisPressureMirrorOptionWidget");

    m_horizontalMirror = new QCheckBox(i18n("Horizontally"));
    m_horizontalMirror->setChecked(false);
    m_verticalMirror = new QCheckBox(i18n("Vertically"));
    m_verticalMirror->setChecked(false);

    connect(m_horizontalMirror, SIGNAL(toggled(bool)), SLOT(horizontalMirrorChanged(bool)));
    connect(m_verticalMirror, SIGNAL(toggled(bool)), SLOT(verticalMirrorChanged(bool)));

    QHBoxLayout* hl = new QHBoxLayout;
    hl->addWidget(m_horizontalMirror);
    hl->addWidget(m_verticalMirror);

    QWidget* page = new QWidget;
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addLayout(hl);
    pageLayout->addWidget(curveWidget());

    setConfigurationPage(page);
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



