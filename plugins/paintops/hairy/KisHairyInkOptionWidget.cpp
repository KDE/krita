/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHairyInkOptionWidget.h"

#include <lager/constant.hpp>
#include "ui_wdgInkOptions.h"

#include "KisHairyInkOptionModel.h"
#include "KisCurveWidgetConnectionHelper.h"
#include <KisSpinBoxI18nHelper.h>

namespace {

class KisInkOptionsWidget: public QWidget, public Ui::WdgInkOptions
{
public:
    KisInkOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        pressureSlider->setRange(0, 100, 0);
        KisSpinBoxI18nHelper::setText(pressureSlider, i18nc("{n} is the number value, % is the percent sign", "{n}%"));

        bristleLengthSlider->setRange(0, 100, 0);
        KisSpinBoxI18nHelper::setText(bristleLengthSlider,
                                      i18nc("{n} is the number value, % is the percent sign", "{n}%"));

        bristleInkAmountSlider->setRange(0, 100, 0);
        KisSpinBoxI18nHelper::setText(bristleInkAmountSlider,
                                      i18nc("{n} is the number value, % is the percent sign", "{n}%"));

        inkDepletionSlider->setRange(0, 100, 0);
        KisSpinBoxI18nHelper::setText(inkDepletionSlider,
                                      i18nc("{n} is the number value, % is the percent sign", "{n}%"));
    }
};

}


struct KisHairyInkOptionWidget::Private
{
    Private(lager::cursor<KisHairyInkOptionData> optionData)
        : model(optionData)
    {
    }

    KisHairyInkOptionModel model;
};

KisHairyInkOptionWidget::KisHairyInkOptionWidget(lager::cursor<KisHairyInkOptionData> optionData)
    : KisPaintOpOption(i18n("Ink depletion"), KisPaintOpOption::COLOR, optionData[&KisHairyInkOptionData::inkDepletionEnabled])
    , m_d(new Private(optionData))
{
    KisInkOptionsWidget *widget = new KisInkOptionsWidget();
    setObjectName("KisHairyInkOption");

    using namespace KisWidgetConnectionUtils;

    connectControl(widget->inkAmountSpinBox,  &m_d->model, "inkAmount");
    connectControl(widget->saturationCBox, &m_d->model, "useSaturation");
    connectControl(widget->opacityCBox, &m_d->model, "useOpacity");
    connectControl(widget->useWeightCHBox, &m_d->model, "useWeights");
    connectControl(widget->pressureSlider, &m_d->model, "pressureWeight");
    connectControl(widget->bristleLengthSlider, &m_d->model, "bristleLengthWeight");
    connectControl(widget->bristleInkAmountSlider, &m_d->model, "bristleInkAmountWeight");
    connectControl(widget->inkDepletionSlider, &m_d->model, "inkDepletionWeight");
    connectControl(widget->inkCurve, &m_d->model, "inkDepletionCurve");
    connectControl(widget->soakInkCBox, &m_d->model, "useSoakInk");

    m_d->model.optionData.bind(std::bind(&KisHairyInkOptionWidget::emitSettingChanged, this));

    setConfigurationPage(widget);
}

KisHairyInkOptionWidget::~KisHairyInkOptionWidget()
{
}

void KisHairyInkOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisHairyInkOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisHairyInkOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisHairyInkOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
