/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisScatterOptionWidget.h"

#include <KisLager.h>

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <KisScatterOptionModel.h>
#include <KisWidgetConnectionUtils.h>


struct KisScatterOptionWidget::Private
{
    Private(lager::cursor<KisScatterOptionData> optionData)
        : model(optionData.zoom(
                    kislager::lenses::to_base<KisScatterOptionMixIn>)
                )
    {
    }

    KisScatterOptionModel model;
};

KisScatterOptionWidget::KisScatterOptionWidget(lager::cursor<KisScatterOptionData> optionData)
    : KisScatterOptionWidget(optionData, KisPaintOpOption::GENERAL)
{
}

KisScatterOptionWidget::KisScatterOptionWidget(lager::cursor<KisScatterOptionData> optionData, PaintopCategory categoryOverride)
    : KisCurveOptionWidget(optionData.zoom(kislager::lenses::to_base<KisCurveOptionDataCommon>), categoryOverride)
    , m_d(new Private(optionData))
{
    using namespace KisWidgetConnectionUtils;

    QWidget *page = new QWidget;

    QCheckBox *axisX = new QCheckBox(i18n("Axis X"), page);
    QCheckBox *axisY = new QCheckBox(i18n("Axis Y"), page);
    QLabel* scatterLbl = new QLabel(i18n("Scatter amount"), page);

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(scatterLbl);
    hl->addWidget(axisX);
    hl->addWidget(axisY);

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addLayout(hl);
    pageLayout->addWidget(configurationPage());

    setConfigurationPage(page);

    connectControl(axisX, &m_d->model, "axisX");
    connectControl(axisY, &m_d->model, "axisY");

    m_d->model.scatterOptionData.bind(std::bind(&KisScatterOptionWidget::emitSettingChanged, this));
}

KisScatterOptionWidget::~KisScatterOptionWidget()
{

}

void KisScatterOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOptionWidget::writeOptionSetting(setting);
    m_d->model.scatterOptionData->write(setting.data());
}

void KisScatterOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisScatterOptionMixIn data = *m_d->model.scatterOptionData;
    data.read(setting.data());
    m_d->model.scatterOptionData.set(data);

    KisCurveOptionWidget::readOptionSetting(setting);
}
