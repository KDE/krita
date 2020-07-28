/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <klocalizedstring.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

#include "kis_smudge_option_widget.h"
#include "kis_smudge_option.h"


KisSmudgeOptionWidget::KisSmudgeOptionWidget()
    : KisCurveOptionWidget(new KisSmudgeOption(), i18n("0.0"), i18n("1.0"))
{
    setObjectName("KisSmudgeOptionWidget");

    mCbSmudgeMode = new QComboBox();
    mCbSmudgeMode->addItem(i18n("Smearing"), KisSmudgeOption::SMEARING_MODE);
    mCbSmudgeMode->addItem("dulling-placeholder" , KisSmudgeOption::DULLING_MODE);

    mChkSmearAlpha = new QCheckBox();
    mChkUseNewEngine = new QCheckBox();
    // the text for the second item is initialized here
    updateBrushPierced(false);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(i18n("Smudge mode:"), mCbSmudgeMode);
    formLayout->addRow(i18n("Smear alpha:"), mChkSmearAlpha);
    formLayout->addRow(i18n("Use New Engine (mandatory for Color, Lightness, and Gradient brushes):"), mChkUseNewEngine);

    QWidget     *page = new QWidget();
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);

    pageLayout->addLayout(formLayout);
    pageLayout->addWidget(curveWidget());


    KisCurveOptionWidget::setConfigurationPage(page);

    connect(mCbSmudgeMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentIndexChanged(int)));
    connect(mChkSmearAlpha, SIGNAL(toggled(bool)), SLOT(slotSmearAlphaChanged(bool)));
    connect(mChkUseNewEngine, SIGNAL(toggled(bool)), SLOT(slotUseNewEngineChanged(bool)));
}

void KisSmudgeOptionWidget::slotCurrentIndexChanged(int index)
{
    static_cast<KisSmudgeOption*>(curveOption())->setMode((KisSmudgeOption::Mode)index);
    emitSettingChanged();
}

void KisSmudgeOptionWidget::slotSmearAlphaChanged(bool value)
{
    static_cast<KisSmudgeOption*>(curveOption())->setSmearAlpha(value);
    emitSettingChanged();
}

void KisSmudgeOptionWidget::slotUseNewEngineChanged(bool value)
{
    static_cast<KisSmudgeOption*>(curveOption())->setUseNewEngine(value);
    emitSettingChanged();
}

void KisSmudgeOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOptionWidget::readOptionSetting(setting);

    KisSmudgeOption::Mode mode = static_cast<KisSmudgeOption*>(curveOption())->getMode();
    mCbSmudgeMode->setCurrentIndex(mode == KisSmudgeOption::SMEARING_MODE ? 0 : 1);

    const bool smearAlpha = static_cast<KisSmudgeOption*>(curveOption())->getSmearAlpha();
    mChkSmearAlpha->setChecked(smearAlpha);

    const bool useNewEngine = static_cast<KisSmudgeOption*>(curveOption())->getUseNewEngine();
    mChkUseNewEngine->setChecked(useNewEngine);
}

void KisSmudgeOptionWidget::updateBrushPierced(bool pierced)
{
    QString dullingText = i18n("Dulling");
    QString toolTip;

    if (pierced) {
        dullingText += i18n(" (caution, pierced brush!)");
        toolTip = i18nc("@info:tooltip", "This brush has transparent pixels in its center. \"Dulling\" mode may give unstable results. Consider using \"Smearing\" mode instead.");
    }

    mCbSmudgeMode->setItemText(1, dullingText);
    mCbSmudgeMode->setToolTip(toolTip);
}

void KisSmudgeOptionWidget::setUseNewEngineCheckboxEnabled(bool enabled) {
    mChkUseNewEngine->setEnabled(enabled);
}

void KisSmudgeOptionWidget::setUseNewEngine(bool useNew) {
    mChkUseNewEngine->setChecked(useNew);
}
