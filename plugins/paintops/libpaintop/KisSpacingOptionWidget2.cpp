/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSpacingOptionWidget2.h"

#include <KisZug.h>

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <KisSpacingOptionModel.h>
#include <KisWidgetConnectionUtils.h>
#include <KisZug.h>

struct KisSpacingOptionWidget2::Private
{
    Private(lager::cursor<KisSpacingOptionData> optionData)
        : model(optionData.zoom(
                    kiszug::lenses::to_base<KisSpacingOptionMixIn>)
                )
    {
    }

    KisSpacingOptionModel model;
};

KisSpacingOptionWidget2::KisSpacingOptionWidget2(lager::cursor<KisSpacingOptionData> optionData)
    : KisCurveOptionWidget2(optionData.zoom(kiszug::lenses::to_base<KisCurveOptionData>), KisPaintOpOption::GENERAL)
    , m_d(new Private(optionData))
{
    using namespace KisWidgetConnectionUtils;

    QWidget *page = new QWidget;

    QCheckBox *isotropicSpacing = new QCheckBox(i18n("Isotropic Spacing"), page);
    QCheckBox *useSpacingUpdates = new QCheckBox(i18n("Update Between Dabs"), page);

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(isotropicSpacing);
    hl->addWidget(useSpacingUpdates);

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addLayout(hl);
    pageLayout->addWidget(configurationPage());

    setConfigurationPage(page);

    connectControl(isotropicSpacing, &m_d->model, "isotropicSpacing");
    connectControl(useSpacingUpdates, &m_d->model, "useSpacingUpdates");

    m_d->model.spacingOptionData.bind(std::bind(&KisSpacingOptionWidget2::emitSettingChanged, this));
}

KisSpacingOptionWidget2::~KisSpacingOptionWidget2()
{
}

void KisSpacingOptionWidget2::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOptionWidget2::writeOptionSetting(setting);
    m_d->model.spacingOptionData->write(setting.data());
}

void KisSpacingOptionWidget2::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSpacingOptionMixIn data = *m_d->model.spacingOptionData;
    data.read(setting.data());
    m_d->model.spacingOptionData.set(data);

    KisCurveOptionWidget2::readOptionSetting(setting);
}
