/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisLightnessStrengthOptionWidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <KisLager.h>
#include <KisStandardOptionData.h>


struct KisLightnessStrengthOptionWidget::Private
{
    Private(lager::reader<bool> lightnessModeEnabled)
        : warningLabelVisible{lightnessModeEnabled.map(std::logical_not{})}
    {
    }

    lager::reader<bool> warningLabelVisible;
};

KisLightnessStrengthOptionWidget::KisLightnessStrengthOptionWidget(lager::cursor<KisLightnessStrengthOptionData> optionData, lager::reader<bool> lightnessModeEnabled)
    : KisCurveOptionWidget(optionData.zoom(kislager::lenses::to_base<KisCurveOptionDataCommon>), KisPaintOpOption::GENERAL, lightnessModeEnabled)
    , m_d(new Private(lightnessModeEnabled))
{
    QWidget* page = new QWidget;

    QLabel *enabledLabel = new QLabel(i18n("Disabled: brush must be in Lightness mode for this option to apply"), page);
    enabledLabel->setEnabled(true);
    enabledLabel->setAlignment(Qt::AlignHCenter);

    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addWidget(enabledLabel);
    pageLayout->addWidget(configurationPage());

    m_d->warningLabelVisible.bind(std::bind(&QWidget::setVisible, enabledLabel, std::placeholders::_1));

    setConfigurationPage(page);
}

KisLightnessStrengthOptionWidget::~KisLightnessStrengthOptionWidget()
{
}
