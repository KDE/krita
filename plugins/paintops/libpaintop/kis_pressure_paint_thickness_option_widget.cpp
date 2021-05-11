/*
 * SPDX-FileCopyrightText: 2021 Peter Schatz <voronwe13@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_pressure_paint_thickness_option_widget.h"

#include <QWidget>
#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>

#include <klocalizedstring.h>

#include "kis_pressure_paint_thickness_option.h"

KisPressurePaintThicknessOptionWidget::KisPressurePaintThicknessOptionWidget()
    : KisCurveOptionWidget(new KisPressurePaintThicknessOption(), i18n("0%"), i18n("100%"))
{
    setObjectName("KisPressurePaintThicknessOptionWidget");

    m_enabledLabel = new QLabel(i18n("Disabled: brush must be in Lightness mode for this option to apply"));
    m_enabledLabel->setEnabled(true);
    m_enabledLabel->setAlignment(Qt::AlignHCenter);

    m_cbThicknessMode = new QComboBox();
    m_cbThicknessMode->addItem(i18n("Smear existing paint thickness"), KisPressurePaintThicknessOption::SMUDGE);
    m_cbThicknessMode->addItem(i18n("Overwrite (Smooth out when low) existing paint thickness"), KisPressurePaintThicknessOption::OVERWRITE);
    m_cbThicknessMode->addItem(i18n("Paint over existing paint thickness (controlled by Smudge Length)"), KisPressurePaintThicknessOption::OVERLAY);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(i18n("Paint Thickness Mode:"), m_cbThicknessMode);
    formLayout->addRow(new QLabel(i18n("Describes how the brush's paint thickness interacts with existing thick paint, especially at low values.")));

    QWidget* page = new QWidget;
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addWidget(m_enabledLabel);
    pageLayout->addLayout(formLayout);
    pageLayout->addWidget(curveWidget());

    setConfigurationPage(page);

    connect(m_cbThicknessMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentIndexChanged(int)));
}

void KisPressurePaintThicknessOptionWidget::setEnabled(bool enabled)
{
    KisCurveOptionWidget::setEnabled(enabled);
    m_enabledLabel->setVisible(!enabled);
}

void KisPressurePaintThicknessOptionWidget::slotCurrentIndexChanged(int index)
{
    static_cast<KisPressurePaintThicknessOption*>(curveOption())->setThicknessMode((KisPressurePaintThicknessOption::ThicknessMode)index);
    emitSettingChanged();
}



