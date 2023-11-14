/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSketchOpOptionWidget.h"

#include <lager/constant.hpp>
#include "ui_wdgsketchoptions.h"

#include "KisSketchOpOptionModel.h"
#include <KisDoubleSpinBoxPluralHelper.h>
#include <kis_paintop_lod_limitations.h>

namespace {


class KisSketchOpWidget: public QWidget, public Ui::WdgSketchOptions
{
public:
    KisSketchOpWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        lineWidthSPBox->setRange(1, 100, 0);
        lineWidthSPBox->setSuffix(i18n(" px"));
        lineWidthSPBox->setExponentRatio(1.5);

        offsetSPBox->setRange(0.0, 200.0, 0);
        KisDoubleSpinBoxPluralHelper::install(offsetSPBox, [](double value) {
            return i18nc("{n} is the number value, % is the percent sign", "{n}%", value);
        });

        densitySPBox->setRange(0.0, 100.0, 0);
        KisDoubleSpinBoxPluralHelper::install(densitySPBox, [](double value) {
            return i18nc("{n} is the number value, % is the percent sign", "{n}%", value);
        });
    }
};


}


struct KisSketchOpOptionWidget::Private
{
    Private(lager::cursor<KisSketchOpOptionData> optionData)
        : model(optionData)
    {
    }

    KisSketchOpOptionModel model;
};


KisSketchOpOptionWidget::KisSketchOpOptionWidget(lager::cursor<KisSketchOpOptionData> optionData)
    : KisPaintOpOption(i18n("Brush size"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{

	KisSketchOpWidget *widget = new KisSketchOpWidget();
	setObjectName("KisSketchOpOption");

	m_checkable = false;

    using namespace KisWidgetConnectionUtils;

    connectControl(widget->offsetSPBox, &m_d->model, "offset");
    connectControl(widget->lineWidthSPBox, &m_d->model, "lineWidth");
    connectControl(widget->densitySPBox, &m_d->model, "probability");
    connectControl(widget->simpleModeCHBox, &m_d->model, "simpleMode");
    connectControl(widget->connectionCHBox, &m_d->model, "makeConnection");
    connectControl(widget->magnetifyCHBox, &m_d->model, "magnetify");
    connectControl(widget->randomRGBCHbox, &m_d->model, "randomRGB");
    connectControl(widget->randomOpacityCHbox, &m_d->model, "randomOpacity");
    connectControl(widget->distanceDensityCHBox, &m_d->model, "distanceDensity");
    connectControl(widget->distanceOpacityCHbox, &m_d->model, "distanceOpacity");
    connectControl(widget->antiAliasingCHBox, &m_d->model, "antiAliasing");

    m_d->model.optionData.bind(std::bind(&KisSketchOpOptionWidget::emitSettingChanged, this));

    setConfigurationPage(widget);
}

KisSketchOpOptionWidget::~KisSketchOpOptionWidget()
{
}

void KisSketchOpOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisSketchOpOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisSketchOpOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSketchOpOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
