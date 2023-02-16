/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPaintThicknessOptionWidget.h"

#include <QLabel>
#include <QComboBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <KisLager.h>
#include <KisPaintThicknessOptionModel.h>
#include <KisWidgetConnectionUtils.h>

struct KisPaintThicknessOptionWidget::Private
{
    Private(lager::cursor<KisPaintThicknessOptionData> optionData,
            lager::reader<bool> lightnessModeEnabled)
        : model{optionData.zoom(kislager::lenses::to_base<KisPaintThicknessOptionMixIn>)},
          warningLabelVisible{lightnessModeEnabled.map(std::logical_not{})}
    {
    }

    KisPaintThicknessOptionModel model;
    lager::reader<bool> warningLabelVisible;
};

KisPaintThicknessOptionWidget::KisPaintThicknessOptionWidget(lager::cursor<KisPaintThicknessOptionData> optionData,
                                                             lager::reader<bool> lightnessModeEnabled)
    : KisCurveOptionWidget(optionData.zoom(kislager::lenses::to_base<KisCurveOptionDataCommon>), KisPaintOpOption::GENERAL, lightnessModeEnabled)
    , m_d(new Private(optionData, lightnessModeEnabled))
{
    using namespace KisWidgetConnectionUtils;

    QWidget* page = new QWidget;

    QLabel *enabledLabel = new QLabel(i18n("Disabled: brush must be in Lightness mode for this option to apply"), page);
    enabledLabel->setEnabled(true);
    enabledLabel->setAlignment(Qt::AlignHCenter);

    QComboBox *cmbThicknessMode = new QComboBox(page);
    cmbThicknessMode->addItem(i18n("Overwrite (Smooth out when low) existing paint thickness"));
    cmbThicknessMode->addItem(i18n("Paint over existing paint thickness (controlled by Smudge Length)"));

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(i18n("Paint Thickness Mode:"), cmbThicknessMode);
    formLayout->addRow(new QLabel(i18n("Describes how the brush's paint thickness interacts with existing thick paint, especially at low values.")));

    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addWidget(enabledLabel);
    pageLayout->addLayout(formLayout);
    pageLayout->addWidget(configurationPage());

    m_d->warningLabelVisible.bind(std::bind(&QWidget::setVisible, enabledLabel, std::placeholders::_1));

    setConfigurationPage(page);

    connectControl(cmbThicknessMode, &m_d->model, "mode");

    m_d->model.optionData.bind(std::bind(&KisPaintThicknessOptionWidget::emitSettingChanged, this));
}

KisPaintThicknessOptionWidget::~KisPaintThicknessOptionWidget()
{
}

void KisPaintThicknessOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOptionWidget::writeOptionSetting(setting);
    m_d->model.optionData->write(setting.data());
}

void KisPaintThicknessOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisPaintThicknessOptionMixIn data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);

    KisCurveOptionWidget::readOptionSetting(setting);
}
