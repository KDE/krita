/*
 * Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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
