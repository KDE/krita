/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisExperimentOpOptionWidget.h"

#include <QButtonGroup>

#include <lager/constant.hpp>
#include "ui_wdgexperimentoptions.h"

#include "KisExperimentOpOptionModel.h"

namespace {


class KisExperimentOpWidget: public QWidget, public Ui::WdgExperimentOptions
{
public:
    KisExperimentOpWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        speed->setRange(0.0, 100.0, 0);
        speed->setSuffix(QChar(Qt::Key_Percent));
        speed->setValue(42.0);
        speed->setSingleStep(1.0);

        smoothThreshold->setRange(0.0, 100.0, 0);
        smoothThreshold->setSuffix(i18n(" px"));
        smoothThreshold->setValue(20.0);
        smoothThreshold->setSingleStep(1.0);

        displaceStrength->setRange(0.0, 100.0, 0);
        displaceStrength->setSuffix(QChar(Qt::Key_Percent));
        displaceStrength->setValue(42.0);
        displaceStrength->setSingleStep(1.0);
    }
};


}


struct KisExperimentOpOptionWidget::Private
{
    Private(lager::cursor<KisExperimentOpOptionData> optionData)
        : model(optionData)
    {
    }

    KisExperimentOpOptionModel model;
};


KisExperimentOpOptionWidget::KisExperimentOpOptionWidget(lager::cursor<KisExperimentOpOptionData> optionData)
    : KisPaintOpOption(i18n("Experiment Option"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
	
	KisExperimentOpWidget *widget = new KisExperimentOpWidget();
	setObjectName("KisExperimentOpOption");
	
	m_checkable = false;
	
    using namespace KisWidgetConnectionUtils;
    connectControl(widget->displaceCHBox, &m_d->model, "isDisplacementEnabled");
    connectControl(widget->displaceStrength, &m_d->model, "displacement");
    
    connectControl(widget->speedCHBox, &m_d->model, "isSpeedEnabled");
    connectControl(widget->speed, &m_d->model, "speed");
    connectControl(widget->smoothCHBox, &m_d->model, "isSmoothingEnabled");
    connectControl(widget->smoothThreshold, &m_d->model, "smoothing");

    connectControl(widget->windingFillCHBox, &m_d->model, "windingFill");
    connectControl(widget->hardEdgeCHBox, &m_d->model, "hardEdge");
    
    QButtonGroup *group = new QButtonGroup(widget);
    group->addButton(widget->patternButton, static_cast<int>(ExperimentFillType::Pattern));
    group->addButton(widget->solidColorButton, static_cast<int>(ExperimentFillType::SolidColor));
    group->setExclusive(true);
    
    connectControl(group, &m_d->model, "fillType");
    
    m_d->model.optionData.bind(std::bind(&KisExperimentOpOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(widget);
}

KisExperimentOpOptionWidget::~KisExperimentOpOptionWidget()
{
}

void KisExperimentOpOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisExperimentOpOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisExperimentOpOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisExperimentOpOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
