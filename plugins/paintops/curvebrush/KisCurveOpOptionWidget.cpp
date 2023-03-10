/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCurveOpOptionWidget.h"

#include <QButtonGroup>

#include <lager/constant.hpp>
#include "ui_wdgcurveoptions.h"

#include "KisCurveOpOptionModel.h"

namespace {


class KisCurveOptionsWidget: public QWidget, public Ui::WdgCurveOptions
{

public:
    KisCurveOptionsWidget(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);
        historySizeSlider->setRange(2, 300);
        historySizeSlider->setValue(30);

        lineWidthSlider->setRange(1, 100);
        lineWidthSlider->setValue(1);
        lineWidthSlider->setSuffix(i18n(" px"));

        curvesOpacitySlider->setRange(0.0, 1.0, 2);
        curvesOpacitySlider->setSingleStep(0.01);
        curvesOpacitySlider->setValue(1.0);
    }
};


}


struct KisCurveOpOptionWidget::Private
{
    Private(lager::cursor<KisCurveOpOptionData> optionData)
        : model(optionData)
    {
    }

    KisCurveOpOptionModel model;
};


KisCurveOpOptionWidget::KisCurveOpOptionWidget(lager::cursor<KisCurveOpOptionData> optionData)
    : KisPaintOpOption(i18nc("Brush settings curve value", "Value"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
	
	KisCurveOptionsWidget *widget = new KisCurveOptionsWidget();
	setObjectName("KisCurveOpOption");
	
	m_checkable = false;
	
    using namespace KisWidgetConnectionUtils;
    connectControl(widget->connectionCHBox, &m_d->model, "curvePaintConnectionLine");
    connectControl(widget->smoothingCHBox, &m_d->model, "curveSmoothing");
    connectControl(widget->historySizeSlider, &m_d->model, "curveStrokeHistorySize");
    connectControl(widget->lineWidthSlider, &m_d->model, "curveLineWidth");
    connectControl(widget->curvesOpacitySlider, &m_d->model, "curveCurvesOpacity");
    
    m_d->model.optionData.bind(std::bind(&KisCurveOpOptionWidget::emitSettingChanged, this));
    
    setConfigurationPage(widget);
}

KisCurveOpOptionWidget::~KisCurveOpOptionWidget()
{
}

void KisCurveOpOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOpOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisCurveOpOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOpOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
