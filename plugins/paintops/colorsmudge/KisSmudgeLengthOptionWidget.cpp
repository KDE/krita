/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSmudgeLengthOptionWidget.h"

#include <KisLager.h>

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <KisSmudgeLengthOptionModel.h>
#include <KisWidgetConnectionUtils.h>


struct KisSmudgeLengthOptionWidget::Private
{
    Private(lager::cursor<KisSmudgeLengthOptionData> optionData,
            lager::reader<bool> _isBrushPierced,
            lager::reader<bool> forceNewEngine)
        : model(optionData.zoom(
                    kislager::lenses::to_base<KisSmudgeLengthOptionMixIn>),
                forceNewEngine
                )
        , isBrushPierced(_isBrushPierced)
    {
    }

    KisSmudgeLengthOptionModel model;
    lager::reader<bool> isBrushPierced;
    QComboBox *cmbSmudgeMode {nullptr};
};

KisSmudgeLengthOptionWidget::KisSmudgeLengthOptionWidget(lager::cursor<KisSmudgeLengthOptionData> optionData,
                                                         lager::reader<bool> isBrushPierced,
                                                         lager::reader<bool> forceNewEngine)
    : KisCurveOptionWidget(optionData.zoom(kislager::lenses::to_base<KisCurveOptionDataCommon>), KisPaintOpOption::GENERAL)
    , m_d(new Private(optionData, isBrushPierced, forceNewEngine))
{
    using namespace KisWidgetConnectionUtils;

    setObjectName("KisSmudgeOptionWidget");

    QWidget *page = new QWidget();

    m_d->cmbSmudgeMode = new QComboBox(page);
    m_d->cmbSmudgeMode->addItem(i18n("Smearing"), KisSmudgeLengthOptionData::SMEARING_MODE);
    m_d->cmbSmudgeMode->addItem("dulling-placeholder" , KisSmudgeLengthOptionData::DULLING_MODE);

    QCheckBox *chkSmearAlpha = new QCheckBox(page);
    QCheckBox *chkUseNewEngine = new QCheckBox(page);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(i18n("Smudge mode:"), m_d->cmbSmudgeMode);
    formLayout->addRow(i18n("Smear alpha:"), chkSmearAlpha);
    formLayout->addRow(i18n("Use new smudge algorithm:"), chkUseNewEngine);
    formLayout->addRow(new QLabel(i18n("(required for Color Image, Lightness Map, and Gradient Map brushes)")));

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);

    pageLayout->addLayout(formLayout);
    pageLayout->addWidget(configurationPage());

    setConfigurationPage(page);

    connectControl(m_d->cmbSmudgeMode, &m_d->model, "mode");
    connectControl(chkSmearAlpha, &m_d->model, "smearAlpha");
    connectControlState(chkUseNewEngine, &m_d->model, "useNewEngineState", "useNewEngine");

    m_d->isBrushPierced.bind(std::bind(&KisSmudgeLengthOptionWidget::updateBrushPierced, this, std::placeholders::_1));
    m_d->model.optionData.bind(std::bind(&KisSmudgeLengthOptionWidget::emitSettingChanged, this));
}

KisSmudgeLengthOptionWidget::~KisSmudgeLengthOptionWidget()
{

}

void KisSmudgeLengthOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOptionWidget::writeOptionSetting(setting);
    m_d->model.backedOptionData().write(setting.data());
}

void KisSmudgeLengthOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisSmudgeLengthOptionMixIn data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);

    KisCurveOptionWidget::readOptionSetting(setting);
}

lager::reader<bool> KisSmudgeLengthOptionWidget::useNewEngine() const
{
    return m_d->model.LAGER_QT(useNewEngineState)[&CheckBoxState::value];
}

void KisSmudgeLengthOptionWidget::updateBrushPierced(bool pierced)
{
    QString dullingText = i18n("Dulling");
    QString toolTip;

    if (pierced) {
        dullingText += i18n(" (caution, pierced brush!)");
        toolTip = i18nc("@info:tooltip", "This brush has transparent pixels in its center. \"Dulling\" mode may give unstable results. Consider using \"Smearing\" mode instead.");
    }

    m_d->cmbSmudgeMode->setItemText(1, dullingText);
    m_d->cmbSmudgeMode->setToolTip(toolTip);
}
