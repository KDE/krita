/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSmudgeOverlayModeOptionWidget.h"

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <kis_paintop_lod_limitations.h>
#include <KisSmudgeOverlayModeOptionModel.h>
#include <KisWidgetConnectionUtils.h>

struct KisSmudgeOverlayModeOptionWidget::Private
{
    Private(lager::cursor<KisSmudgeOverlayModeOptionData> optionData,
            lager::reader<bool> overlayModeAllowed)
        : model(optionData, overlayModeAllowed)
        , warningLabelVisible{overlayModeAllowed.map(std::logical_not{})}
        , lodLimitations(optionData.map(std::mem_fn(&KisSmudgeOverlayModeOptionData::lodLimitations)))
    {
    }

    KisSmudgeOverlayModeOptionModel model;
    lager::reader<bool> warningLabelVisible;
    lager::reader<KisPaintopLodLimitations> lodLimitations;
};


KisSmudgeOverlayModeOptionWidget::KisSmudgeOverlayModeOptionWidget(lager::cursor<KisSmudgeOverlayModeOptionData> optionData,
                                                                   lager::reader<bool> overlayModeAllowed)
    : KisPaintOpOption(i18n("Overlay Mode"),
                       KisPaintOpOption::GENERAL,
                       optionData[&KisSmudgeOverlayModeOptionData::isChecked],
                       overlayModeAllowed)
    , m_d(new Private(optionData, overlayModeAllowed))
{
    using namespace KisWidgetConnectionUtils;

    QWidget *page = new QWidget();

    QLabel *label = new QLabel(
        i18n("Paints on the current layer\n"
             "but uses all layers that are currently visible for smudge input\n"
             "NOTE: This mode is only able to work correctly with a fully opaque background"),
        page);

    label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);


    QLabel *disabledWarningLabel = new QLabel(
        i18n("Disabled: overlay mode is not supported in Lightness mode of the brush"),
        page);

    disabledWarningLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);


    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->addWidget(disabledWarningLabel);
    layout->addWidget(label, 1);

    m_d->warningLabelVisible.bind(std::bind(&QWidget::setVisible, disabledWarningLabel, std::placeholders::_1));

    setConfigurationPage(page);

    m_d->model.optionData.bind(std::bind(&KisSmudgeOverlayModeOptionWidget::emitSettingChanged, this));
}

KisSmudgeOverlayModeOptionWidget::~KisSmudgeOverlayModeOptionWidget()
{
}

void KisSmudgeOverlayModeOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.bakedOptionData().write(setting.data());
}

void KisSmudgeOverlayModeOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSmudgeOverlayModeOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);

}

KisPaintOpOption::OptionalLodLimitationsReader KisSmudgeOverlayModeOptionWidget::lodLimitationsReader() const
{
    return m_d->lodLimitations;
}
