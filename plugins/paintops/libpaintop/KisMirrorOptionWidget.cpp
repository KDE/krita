/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisMirrorOptionWidget.h"

#include <KisLager.h>

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <KisMirrorOptionModel.h>
#include <KisWidgetConnectionUtils.h>


struct KisMirrorOptionWidget::Private
{
    Private(lager::cursor<KisMirrorOptionData> optionData)
        : model(optionData.zoom(
                    kislager::lenses::to_base<KisMirrorOptionMixIn>)
                )
    {
    }

    KisMirrorOptionModel model;
};

KisMirrorOptionWidget::KisMirrorOptionWidget(lager::cursor<KisMirrorOptionData> optionData)
    : KisMirrorOptionWidget(optionData, KisPaintOpOption::GENERAL)
{
}

KisMirrorOptionWidget::KisMirrorOptionWidget(lager::cursor<KisMirrorOptionData> optionData, PaintopCategory categoryOverride)
    : KisCurveOptionWidget(optionData.zoom(kislager::lenses::to_base<KisCurveOptionDataCommon>), categoryOverride)
    , m_d(new Private(optionData))
{
    using namespace KisWidgetConnectionUtils;

    QWidget *page = new QWidget;

    QCheckBox *horizontalMirror = new QCheckBox(i18n("Horizontally"), page);
    QCheckBox *verticalMirror = new QCheckBox(i18n("Vertically"), page);

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(horizontalMirror);
    hl->addWidget(verticalMirror);

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addLayout(hl);
    pageLayout->addWidget(configurationPage());

    setConfigurationPage(page);

    connectControl(horizontalMirror, &m_d->model, "enableHorizontalMirror");
    connectControl(verticalMirror, &m_d->model, "enableVerticalMirror");

    m_d->model.mirrorOptionData.bind(std::bind(&KisMirrorOptionWidget::emitSettingChanged, this));
}

KisMirrorOptionWidget::~KisMirrorOptionWidget()
{
}

void KisMirrorOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOptionWidget::writeOptionSetting(setting);
    m_d->model.mirrorOptionData->write(setting.data());
}

void KisMirrorOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisMirrorOptionMixIn data = *m_d->model.mirrorOptionData;
    data.read(setting.data());
    m_d->model.mirrorOptionData.set(data);

    KisCurveOptionWidget::readOptionSetting(setting);
}
