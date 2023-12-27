/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisParticleOpOptionWidget.h"

#include <lager/constant.hpp>
#include "ui_wdgparticleoptions.h"

#include "KisParticleOpOptionModel.h"

namespace {


class KisParticleOpWidget: public QWidget, public Ui::WdgParticleOptions
{
public:
    KisParticleOpWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        gravSPBox->setRange(-1.0, 1.0, 3);
        gravSPBox->setSingleStep(0.001);

        dySPBox->setRange(-10.0, 10.0, 2);
        dySPBox->setSingleStep(0.01);

        dxSPBox->setRange(-10.0, 10.0, 2);
        dxSPBox->setSingleStep(0.01);

        weightSPBox->setRange(0.01, 1.0, 2);
        weightSPBox->setSingleStep(0.01);

        particleSpinBox->setRange(1, 500, 0);
        particleSpinBox->setExponentRatio(3.0);

        itersSPBox->setRange(1, 200, 0);
    }
};


}


struct KisParticleOpOptionWidget::Private
{
    Private(lager::cursor<KisParticleOpOptionData> optionData)
        : model(optionData)
    {
    }

    KisParticleOpOptionModel model;
};


KisParticleOpOptionWidget::KisParticleOpOptionWidget(lager::cursor<KisParticleOpOptionData> optionData)
    : KisPaintOpOption(i18n("Brush size"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
	KisParticleOpWidget *widget = new KisParticleOpWidget();
	setObjectName("KisParticleOpOption");

	m_checkable = false;

    using namespace KisWidgetConnectionUtils;

    connectControl(widget->particleSpinBox, &m_d->model, "particleCount");
    connectControl(widget->itersSPBox, &m_d->model, "particleIterations");
    connectControl(widget->gravSPBox, &m_d->model, "particleGravity");
    connectControl(widget->weightSPBox, &m_d->model, "particleWeight");
    connectControl(widget->dxSPBox, &m_d->model, "particleScaleX");
    connectControl(widget->dySPBox, &m_d->model, "particleScaleY");

    m_d->model.optionData.bind(std::bind(&KisParticleOpOptionWidget::emitSettingChanged, this));

    setConfigurationPage(widget);
}

KisParticleOpOptionWidget::~KisParticleOpOptionWidget()
{
}

void KisParticleOpOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisParticleOpOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisParticleOpOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisParticleOpOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
