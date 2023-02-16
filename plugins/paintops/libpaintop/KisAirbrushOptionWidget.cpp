/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisAirbrushOptionWidget.h"

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <KisAirbrushOptionModel.h>
#include <KisWidgetConnectionUtils.h>

#include "ui_wdgairbrush.h"

const qreal MINIMUM_RATE = 1.0;
const qreal MAXIMUM_RATE = 1000.0;
const int RATE_NUM_DECIMALS = 2;
const qreal RATE_EXPONENT_RATIO = 2.0;
const qreal RATE_SINGLE_STEP = 1.0;
const qreal DEFAULT_RATE = 20.0;

class KisAirbrushWidget: public QWidget, public Ui::WdgAirbrush
{
public:
    KisAirbrushWidget(QWidget *parent = 0, bool canIgnoreSpacing = true)
        : QWidget(parent) {
        setupUi(this);

        sliderRate->setRange(MINIMUM_RATE, MAXIMUM_RATE, RATE_NUM_DECIMALS);
        sliderRate->setExponentRatio(RATE_EXPONENT_RATIO);
        sliderRate->setSingleStep(RATE_SINGLE_STEP);
        sliderRate->setValue(DEFAULT_RATE);

        checkBoxIgnoreSpacing->setVisible(canIgnoreSpacing);
        checkBoxIgnoreSpacing->setEnabled(canIgnoreSpacing);
    }
};


struct KisAirbrushOptionWidget::Private
{
    Private(lager::cursor<KisAirbrushOptionData> optionData)
        : model(optionData)
    {
    }

    KisAirbrushOptionModel model;
};


KisAirbrushOptionWidget::KisAirbrushOptionWidget(lager::cursor<KisAirbrushOptionData> optionData, bool canIgnoreSpacing)
    : KisPaintOpOption(i18n("Airbrush"), KisPaintOpOption::COLOR, optionData[&KisAirbrushOptionData::isChecked])
    , m_d(new Private(optionData))
{
    using namespace KisWidgetConnectionUtils;

    KisAirbrushWidget *page = new KisAirbrushWidget(nullptr, canIgnoreSpacing);
    setConfigurationPage(page);

    connectControl(page->sliderRate, &m_d->model, "airbrushRate");
    connectControl(page->checkBoxIgnoreSpacing, &m_d->model, "ignoreSpacing");

    m_d->model.airbrushOptionData.bind(std::bind(&KisAirbrushOptionWidget::emitSettingChanged, this));
}

KisAirbrushOptionWidget::~KisAirbrushOptionWidget()
{
}

void KisAirbrushOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.airbrushOptionData->write(setting.data());
}

void KisAirbrushOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisAirbrushOptionData data = *m_d->model.airbrushOptionData;
    data.read(setting.data());
    m_d->model.airbrushOptionData.set(data);

}
