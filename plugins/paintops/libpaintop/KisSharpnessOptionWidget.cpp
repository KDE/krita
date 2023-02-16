/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSharpnessOptionWidget.h"


#include <KisLager.h>

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <kis_slider_spin_box.h>

#include <KisSharpnessOptionModel.h>
#include <KisWidgetConnectionUtils.h>

struct KisSharpnessOptionWidget::Private
{
    Private(lager::cursor<KisSharpnessOptionData> optionData)
        : model(optionData.zoom(
                    kislager::lenses::to_base<KisSharpnessOptionMixIn>)
                )
    {
    }

    KisSharpnessOptionModel model;
};

KisSharpnessOptionWidget::KisSharpnessOptionWidget(lager::cursor<KisSharpnessOptionData> optionData)
    : KisCurveOptionWidget(optionData.zoom(kislager::lenses::to_base<KisCurveOptionDataCommon>), KisPaintOpOption::GENERAL)
    , m_d(new Private(optionData))
{
    using namespace KisWidgetConnectionUtils;

    QWidget *page = new QWidget;

    QCheckBox *alignOutline = new QCheckBox(("Align the brush preview outline to the pixel grid"), page);

    QLabel* thresholdLbl = new QLabel(i18n("Soften edge:"), page);
    KisSliderSpinBox *softenEdge = new KisSliderSpinBox(page);
    softenEdge->setRange(0, 100);
    softenEdge->setSingleStep(1);

    QHBoxLayout* alignHL = new QHBoxLayout;
    alignHL->setMargin(2);
    alignHL->addWidget(alignOutline);

    QHBoxLayout* softnessHL = new QHBoxLayout;
    softnessHL->setMargin(9);
    softnessHL->addWidget(thresholdLbl);
    softnessHL->addWidget(softenEdge, 1);

    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addLayout(alignHL);
    pageLayout->addLayout(softnessHL);
    pageLayout->addWidget(configurationPage());

    setConfigurationPage(page);

    connectControl(alignOutline, &m_d->model, "alignOutlinePixels");
    connectControl(softenEdge, &m_d->model, "softness");

    m_d->model.sharpnessOptionData.bind(std::bind(&KisSharpnessOptionWidget::emitSettingChanged, this));
}

KisSharpnessOptionWidget::~KisSharpnessOptionWidget()
{
}

void KisSharpnessOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOptionWidget::writeOptionSetting(setting);
    m_d->model.sharpnessOptionData->write(setting.data());
}

void KisSharpnessOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSharpnessOptionMixIn data = *m_d->model.sharpnessOptionData;
    data.read(setting.data());
    m_d->model.sharpnessOptionData.set(data);

    KisCurveOptionWidget::readOptionSetting(setting);
}
