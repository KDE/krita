/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisBrushSizeOptionWidget.h"

#include <lager/constant.hpp>
#include "ui_wdgBrushSizeOptions.h"

#include <KisSpinBoxI18nHelper.h>
#include "KisBrushSizeOptionModel.h"

namespace {


class KisBrushSizeOptionsWidget: public QWidget, public Ui::WdgBrushSizeOptions
{
public:
    KisBrushSizeOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        diameter->setRange(1.0, 1000, 0);
        diameter->setExponentRatio(3.0);
        diameter->setSuffix(i18n(" px"));

        aspectBox->setRange(0.01, 2.0, 2);
        aspectBox->setSingleStep(0.01);
        aspectBox->setExponentRatio(1.0);


        scale->setRange(0.01, 10.0, 2);
        scale->setSingleStep(0.01);

        spacing->setRange(0.01, 5.0, 2);
        spacing->setSingleStep(0.01);


        rotationBox->setDecimals(0);


        densityBox->setRange(0.0, 100.0, 0);
        KisSpinBoxI18nHelper::setText(densityBox, i18nc("{n} is the number value, % is the percent sign", "{n}%"));

        jitterMove->setRange(0.0, 5.0, 2);
        jitterMove->setSingleStep(0.01);
    }
};


}


struct KisBrushSizeOptionWidget::Private
{
    Private(lager::cursor<KisBrushSizeOptionData> optionData)
        : model(optionData)
    {
    }

    KisBrushSizeOptionModel model;
};


KisBrushSizeOptionWidget::KisBrushSizeOptionWidget(lager::cursor<KisBrushSizeOptionData> optionData)
    : KisPaintOpOption(i18n("Brush size"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
	KisBrushSizeOptionsWidget *widget = new KisBrushSizeOptionsWidget();
	setObjectName("KisBrushSizeOption");

	m_checkable = false;

    using namespace KisWidgetConnectionUtils;

    connectControl(widget->diameter, &m_d->model, "brushDiameter");
    connectControl(widget->scale, &m_d->model, "brushScale");
    connectControl(widget->aspectBox, &m_d->model, "brushAspect");
    connectControl(widget->spacing, &m_d->model, "brushSpacing");
    connectControl(widget->rotationBox, &m_d->model, "brushRotation");
    connectControl(widget->densityBox, &m_d->model, "brushDensity");
    connectControl(widget->jitterMove, &m_d->model, "brushJitterMovement");
    connectControl(widget->jitterMoveBox, &m_d->model, "brushJitterMovementEnabled");

    connect(widget->jitterMoveBox, SIGNAL(toggled(bool)), widget->jitterMove, SLOT(setEnabled(bool)));

    m_d->model.optionData.bind(std::bind(&KisBrushSizeOptionWidget::emitSettingChanged, this));

    setConfigurationPage(widget);
}

KisBrushSizeOptionWidget::~KisBrushSizeOptionWidget()
{
}

void KisBrushSizeOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisBrushSizeOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisBrushSizeOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisBrushSizeOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
