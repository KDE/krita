/* This file is part of the KDE project
 * Copyright (C) Silvio Heinrich <plassy@web.de>, (C) 2011
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

#include "kis_pressure_composite_option_widget.h"
#include "kis_pressure_composite_option.h"

#include <kis_slider_spin_box.h>
#include <KoCompositeOp.h>

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QGridLayout>

#include <klocale.h>

const static int MAX_SLIDER_VALUE = 1000;

KisPressureCompositeOptionWidget::KisPressureCompositeOptionWidget()
    : KisCurveOptionWidget(new KisPressureCompositeOption())
{
    QWidget* widget    = new QWidget;
    QLabel*  modeLabel = new QLabel(i18n("Mode: "));
    QLabel*  rateLabel = new QLabel(i18n("Rate: "));
    
    m_compositeOpBox = new QComboBox();
    m_compositeOpBox->addItem(COMPOSITE_OVER);
    m_compositeOpBox->addItem(COMPOSITE_OVERLAY);
    m_compositeOpBox->addItem(COMPOSITE_SCREEN);
    m_compositeOpBox->addItem(COMPOSITE_ADD);
    m_compositeOpBox->addItem(COMPOSITE_SUBTRACT);
    m_compositeOpBox->addItem(COMPOSITE_DIVIDE);
    m_compositeOpBox->addItem(COMPOSITE_BURN);
    m_compositeOpBox->addItem(COMPOSITE_DODGE);
    m_compositeOpBox->addItem(COMPOSITE_COLOR);
    m_compositeOpBox->addItem(COMPOSITE_HARD_LIGHT);
    m_compositeOpBox->addItem(COMPOSITE_SOFT_LIGHT);
    
    m_rateSlider = new KisDoubleSliderSpinBox();
    m_rateSlider->setRange(0.0, 1.0, 2);
    m_rateSlider->setValue(0.3);
    m_rateSlider->setSingleStep(0.01);
    m_rateSlider->setSuffix("%");
    
    connect(m_compositeOpBox, SIGNAL(activated(QString)), this, SLOT(compositeOpChanged(QString)));
    connect(m_rateSlider, SIGNAL(valueChanged(qreal)), this, SLOT(rateChanged(qreal)));
    
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->addWidget(modeLabel, 0, 0);
    gridLayout->addWidget(m_compositeOpBox, 0, 1);
    gridLayout->addWidget(rateLabel, 1, 0);
    gridLayout->addWidget(m_rateSlider, 1, 1);
    gridLayout->setColumnStretch(1, 1);

    QVBoxLayout* vBoxLayout = new QVBoxLayout;
    vBoxLayout->addLayout(gridLayout);
    vBoxLayout->addWidget(curveWidget());

    widget->setLayout(vBoxLayout);
    
    setConfigurationPage(widget);
    
    compositeOpChanged(COMPOSITE_OVER);
    rateChanged(m_rateSlider->value());
}

void KisPressureCompositeOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);
    
    QString compositeOp = static_cast<KisPressureCompositeOption*>(curveOption())->getCompositeOp();
    
    for(int i=0; i<m_compositeOpBox->count(); ++i) {
        if(m_compositeOpBox->itemText(i) == compositeOp) {
            m_compositeOpBox->setCurrentIndex(i);
            break;
        }
    }
    
    m_rateSlider->setValue(static_cast<KisPressureCompositeOption*>(curveOption())->getRate());
}

void KisPressureCompositeOptionWidget::compositeOpChanged(const QString& compositeOp)
{
    static_cast<KisPressureCompositeOption*>(curveOption())->setCompositeOp(compositeOp);
    emit sigSettingChanged();
}

void KisPressureCompositeOptionWidget::rateChanged(qreal rate)
{
    static_cast<KisPressureCompositeOption*>(curveOption())->setRate(rate);
    emit sigSettingChanged();
}
