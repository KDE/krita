/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHatchingOptionsWidget.h"

#include <QButtonGroup>

#include <lager/constant.hpp>
#include "ui_wdghatchingoptions.h"

#include "KisHatchingOptionsModel.h"
#include <kis_paintop_lod_limitations.h>

namespace {


class KisHatchingOptions: public QWidget, public Ui::WdgHatchingOptions
{
public:
    KisHatchingOptions(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        QString px = i18n(" px");

        angleKisAngleSelector           ->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

        //setRange(minimum, maximum, decimals)
        angleKisAngleSelector           -> setRange(-90.0, 90.0);
        angleKisAngleSelector           -> setDecimals(1);
        separationKisDoubleSliderSpinBox-> setRange(1.0, 30.0, 1);
        thicknessKisDoubleSliderSpinBox -> setRange(1.0, 30.0, 1);
        originXKisDoubleSliderSpinBox   -> setRange(-300, 300, 0);
        originYKisDoubleSliderSpinBox   -> setRange(-300, 300, 0);

        separationKisDoubleSliderSpinBox-> setSingleStep(0.1);
        thicknessKisDoubleSliderSpinBox -> setSingleStep(0.1);

        separationKisDoubleSliderSpinBox-> setSuffix(px);
        thicknessKisDoubleSliderSpinBox -> setSuffix(px);
        originXKisDoubleSliderSpinBox   -> setSuffix(px);
        originYKisDoubleSliderSpinBox   -> setSuffix(px);
    }
};


}


struct KisHatchingOptionsWidget::Private
{
    Private(lager::cursor<KisHatchingOptionsData> optionData)
        : model(optionData)
    {
    }

    KisHatchingOptionsModel model;
};


KisHatchingOptionsWidget::KisHatchingOptionsWidget(lager::cursor<KisHatchingOptionsData> optionData)
    : KisPaintOpOption(i18n("Hatching options"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{

	KisHatchingOptions *widget = new KisHatchingOptions();
	setObjectName("KisHatchingOptions");

	m_checkable = false;

    using namespace KisWidgetConnectionUtils;

    connectControl(widget->angleKisAngleSelector, &m_d->model, "angle");
    connectControl(widget->separationKisDoubleSliderSpinBox, &m_d->model, "separation");
    connectControl(widget->thicknessKisDoubleSliderSpinBox, &m_d->model, "thickness");
    connectControl(widget->originXKisDoubleSliderSpinBox, &m_d->model, "originX");
    connectControl(widget->originYKisDoubleSliderSpinBox, &m_d->model, "originY");

    QButtonGroup *group = new QButtonGroup(widget);
    group->addButton(widget->noCrosshatchingRadioButton, static_cast<int>(CrosshatchingType::NoCrosshatching));
    group->addButton(widget->perpendicularRadioButton, static_cast<int>(CrosshatchingType::Perpendicular));
    group->addButton(widget->minusThenPlusRadioButton, static_cast<int>(CrosshatchingType::MinusThenPlus));
    group->addButton(widget->plusThenMinusRadioButton, static_cast<int>(CrosshatchingType::PlusThenMinus));
    group->addButton(widget->moirePatternRadioButton, static_cast<int>(CrosshatchingType::MoirePattern));
    group->setExclusive(true);
    connectControl(group, &m_d->model, "crosshatchingStyle");

    connectControl(widget->separationIntervalSpinBox, &m_d->model, "separationIntervals");

    m_d->model.optionData.bind(std::bind(&KisHatchingOptionsWidget::emitSettingChanged, this));

    setConfigurationPage(widget);
}

KisHatchingOptionsWidget::~KisHatchingOptionsWidget()
{
}

void KisHatchingOptionsWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisHatchingOptionsData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisHatchingOptionsWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisHatchingOptionsData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
