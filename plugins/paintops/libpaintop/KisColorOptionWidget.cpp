/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisColorOptionWidget.h"

#include <QButtonGroup>
#include <QMetaProperty>

#include <lager/constant.hpp>
#include "ui_wdgcoloroptions.h"

#include "KisColorOptionModel.h"

namespace {

class KisColorOptionsWidgetUI: public QWidget, public Ui::WdgColorOptions
{
public:
    KisColorOptionsWidgetUI(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        hueSlider->setRange(-180, 180);
        hueSlider->setValue(0);

        saturationSlider->setRange(-100, 100);
        saturationSlider->setValue(0);

        valueSlider->setRange(-100, 100);
        valueSlider->setValue(0);
    }
};

}

struct KisColorOptionWidget::Private
{
    Private(lager::cursor<KisColorOptionData> optionData)
        : model(optionData)
    {
    }

    KisColorOptionModel model;
    KisColorOptionsWidgetUI* options {0};
};

KisColorOptionWidget::KisColorOptionWidget(lager::cursor<KisColorOptionData> optionData)
    : KisPaintOpOption(i18n("Color options"), KisPaintOpOption::COLOR, true)
    , m_d(new Private(optionData))
{
	m_d->options = new KisColorOptionsWidgetUI();
	setObjectName("KisColorOption");
	
	m_checkable = false;
	
    using namespace KisWidgetConnectionUtils;
    
    m_d->options->hueSlider->setEnabled(m_d->model.useRandomHSV());
    m_d->options->saturationSlider->setEnabled(m_d->model.useRandomHSV());
    m_d->options->valueSlider->setEnabled(m_d->model.useRandomHSV());
    connect(&m_d->model, &KisColorOptionModel::useRandomHSVChanged, m_d->options->hueSlider, &KisSliderSpinBox::setEnabled);
    connect(&m_d->model, &KisColorOptionModel::useRandomHSVChanged, m_d->options->saturationSlider, &KisSliderSpinBox::setEnabled);
    connect(&m_d->model, &KisColorOptionModel::useRandomHSVChanged, m_d->options->valueSlider, &KisSliderSpinBox::setEnabled);
    
    connectControl(m_d->options->randomOpacityCHBox, &m_d->model, "useRandomOpacity");
    connectControl(m_d->options->randomHSVCHBox, &m_d->model, "useRandomHSV");
    
    connectControl(m_d->options->hueSlider, &m_d->model, "hue");
    connectControl(m_d->options->saturationSlider, &m_d->model, "saturation");
    connectControl(m_d->options->valueSlider, &m_d->model, "value");
    
    connectControl(m_d->options->sampleInputCHBox, &m_d->model, "sampleInputColor");
    connectControl(m_d->options->colorPerParticleCHBox, &m_d->model, "colorPerParticle");
    
    connectControl(m_d->options->fillBackgroundCHBox, &m_d->model, "fillBackground");
    connectControl(m_d->options->mixBgColorCHBox, &m_d->model, "mixBgColor");
    
    
    m_d->model.optionData.bind(std::bind(&KisColorOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(m_d->options);
}

KisColorOptionWidget::~KisColorOptionWidget()
{
}

void KisColorOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisColorOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisColorOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisColorOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
