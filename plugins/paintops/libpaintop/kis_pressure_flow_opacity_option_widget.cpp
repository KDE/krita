/*
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <klocalizedstring.h>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QPushButton>

#include "kis_pressure_flow_opacity_option_widget.h"
#include "kis_curve_option_widget.h"
#include "kis_node.h"
#include <kis_slider_spin_box.h>

#include "ui_wdgflowopacityoption.h"

KisFlowOpacityOptionWidget::KisFlowOpacityOptionWidget():
    KisCurveOptionWidget(new KisFlowOpacityOption(0), i18n("Transparent"), i18n("Opaque"), true)
{
    setObjectName("KisFlowOpacityOptionWidget");

    QWidget* widget = new QWidget();

    Ui_wdgFlowOpacityOption ui;
    ui.setupUi(widget);
    ui.layout->addWidget(curveWidget());

    m_opacitySlider = ui.opacitySlider;
    m_opacitySlider->setRange(0.0, 100.0, 0);
    m_opacitySlider->setValue(100);
    m_opacitySlider->setPrefix("Opacity: ");
    m_opacitySlider->setSuffix(i18n("%"));

    setConfigurationPage(widget);

    connect(m_opacitySlider, SIGNAL(valueChanged(qreal)), SLOT(slotSliderValueChanged()));
}

void KisFlowOpacityOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    KisFlowOpacityOption* option = static_cast<KisFlowOpacityOption*>(curveOption());

    m_opacitySlider->blockSignals(true);
    m_opacitySlider->setValue(option->getStaticOpacity()*100);
    m_opacitySlider->blockSignals(false);
}

void KisFlowOpacityOptionWidget::slotSliderValueChanged()
{
    KisFlowOpacityOption* option = static_cast<KisFlowOpacityOption*>(curveOption());
    option->setOpacity(m_opacitySlider->value()/100.0);

    emitSettingChanged();
}
