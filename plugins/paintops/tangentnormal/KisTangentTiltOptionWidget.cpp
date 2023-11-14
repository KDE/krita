/*
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * 
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisTangentTiltOptionWidget.h"

#include <QButtonGroup>

#include <lager/constant.hpp>
#include "ui_wdgtangenttiltoption.h"

#include "KisTangentTiltOptionModel.h"
#include <KisDoubleSpinBoxPluralHelper.h>

namespace {


class KisTangentTiltWidget: public QWidget, public Ui::WdgTangentTiltOptions
{
public:
    KisTangentTiltWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        sliderElevationSensitivity->setRange(0, 100, 0);
        KisDoubleSpinBoxPluralHelper::install(sliderElevationSensitivity, [](double value) {
            return i18nc("{n} is the number value, % is the percent sign", "{n}%", value);
        });

        sliderMixValue->setRange(0, 100, 0);
        KisDoubleSpinBoxPluralHelper::install(sliderMixValue, [](double value) {
            return i18nc("{n} is the number value, % is the percent sign", "{n}%", value);
        });

        sliderMixValue->setVisible(false);
    }
};


}


struct KisTangentTiltOptionWidget::Private
{
    Private(lager::cursor<KisTangentTiltOptionData> optionData)
        : model(optionData)
    {
    }

    KisTangentTiltOptionModel model;
};


KisTangentTiltOptionWidget::KisTangentTiltOptionWidget(lager::cursor<KisTangentTiltOptionData> optionData)
    : KisPaintOpOption(i18n("Tangent Tilt"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
	
	KisTangentTiltWidget *widget = new KisTangentTiltWidget();
	setObjectName("KisTangentTiltOption");
	
	m_checkable = false;
	
    using namespace KisWidgetConnectionUtils;

    connectControl(widget->comboRed, &m_d->model, "redChannel");
    connectControl(widget->comboGreen, &m_d->model, "greenChannel");
    connectControl(widget->comboBlue, &m_d->model, "blueChannel");

    connect(widget->comboRed, SIGNAL(currentIndexChanged(int)), widget->TangentTiltPreview, SLOT(setRedChannel(int)));
    connect(widget->comboGreen, SIGNAL(currentIndexChanged(int)), widget->TangentTiltPreview, SLOT(setGreenChannel(int)));
    connect(widget->comboBlue, SIGNAL(currentIndexChanged(int)), widget->TangentTiltPreview, SLOT(setBlueChannel(int)));

    QButtonGroup *group = new QButtonGroup(widget);
    group->addButton(widget->optionTilt, static_cast<int>(TangentTiltDirectionType::Tilt));
    group->addButton(widget->optionDirection, static_cast<int>(TangentTiltDirectionType::Direction));
    group->addButton(widget->optionRotation, static_cast<int>(TangentTiltDirectionType::Rotation));
    group->addButton(widget->optionMix, static_cast<int>(TangentTiltDirectionType::Mix));
    group->setExclusive(true);
    connectControl(group, &m_d->model, "directionType");

    connectControl(widget->sliderElevationSensitivity, &m_d->model, "elevationSensitivity");
    connectControl(widget->sliderMixValue, &m_d->model, "mixValue");
    
    m_d->model.optionData.bind(std::bind(&KisTangentTiltOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(widget);
}

KisTangentTiltOptionWidget::~KisTangentTiltOptionWidget()
{
}

void KisTangentTiltOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisTangentTiltOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisTangentTiltOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisTangentTiltOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
