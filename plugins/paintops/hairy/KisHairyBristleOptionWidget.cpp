/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHairyBristleOptionWidget.h"

#include <lager/constant.hpp>
#include "ui_wdgbristleoptions.h"

#include "KisHairyBristleOptionModel.h"

#include <KisDoubleSpinBoxPluralHelper.h>
#include <kis_paintop_lod_limitations.h>

namespace {


class KisBristleOptionsWidget: public QWidget, public Ui::WdgBristleOptions
{
public:
    KisBristleOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        scaleBox->setRange(-10.0, 10.0, 2);
        scaleBox->setSingleStep(0.01);

        rndBox->setRange(-10.0, 10.0, 2);
        rndBox->setSingleStep(0.01);

        shearBox->setRange(-2.0, 2.0, 2);
        shearBox->setSingleStep(0.01);

        densityBox->setRange(0.0, 100.0, 0);
        KisDoubleSpinBoxPluralHelper::install(densityBox, [](double value) {
            return i18nc("{n} is the number value, % is the percent sign", "{n}%", value);
        });
    }
};


}


struct KisHairyBristleOptionWidget::Private
{
    Private(lager::cursor<KisHairyBristleOptionData> optionData)
        : model(optionData)
    {
    }

    KisHairyBristleOptionModel model;
};


KisHairyBristleOptionWidget::KisHairyBristleOptionWidget(lager::cursor<KisHairyBristleOptionData> optionData)
    : KisPaintOpOption(i18n("Bristle options"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{
    KisBristleOptionsWidget *widget = new KisBristleOptionsWidget();
    setObjectName("KisHairyBristleOption");

    m_checkable = false;

    using namespace KisWidgetConnectionUtils;

    connectControl(widget->mousePressureCBox, &m_d->model, "useMousePressure");
    connectControl(widget->thresholdCBox, &m_d->model, "threshold");
    connectControl(widget->scaleBox, &m_d->model, "scaleFactor");
    connectControl(widget->rndBox, &m_d->model, "randomFactor");
    connectControl(widget->shearBox, &m_d->model, "shearFactor");
    connectControl(widget->densityBox, &m_d->model, "densityFactor");
    connectControl(widget->connectedCBox, &m_d->model, "connectedPath");
    connectControl(widget->antialiasCBox, &m_d->model, "antialias");
    connectControl(widget->compositingCBox, &m_d->model, "useCompositing");

    m_d->model.optionData.bind(std::bind(&KisHairyBristleOptionWidget::emitSettingChanged, this));

    setConfigurationPage(widget);
}

KisHairyBristleOptionWidget::~KisHairyBristleOptionWidget()
{
}

void KisHairyBristleOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisHairyBristleOptionData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisHairyBristleOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisHairyBristleOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
