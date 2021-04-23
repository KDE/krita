/*
 *  SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>

 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_texture_chooser.h"
#include "kis_texture_option.h"

#include "widgets/kis_gradient_chooser.h"

KisTextureChooser::KisTextureChooser(KisBrushTextureFlags flags, QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    textureSelectorWidget->setGrayscalePreview(true);
    textureSelectorWidget->setCurrentItem(0);

    scaleSlider->setRange(0.0, 2.0, 2);
    scaleSlider->setValue(1.0);
    scaleSlider->addMultiplier(0.1);
    scaleSlider->addMultiplier(2);
    scaleSlider->addMultiplier(10);

    brightnessSlider->setRange(-1.0, 1.0, 2);
    brightnessSlider->setValue(0.0);
    brightnessSlider->setToolTip(i18n("Makes texture lighter or darker"));

    contrastSlider->setRange(0.0, 2.0, 2);
    contrastSlider->setValue(1.0);

    neutralPointSlider->setRange(0.0, 1.0, 2);
    neutralPointSlider->setValue(0.5);
    neutralPointSlider->setToolTip(i18n("Set gray value to be considered neutral for lightness mode"));

    offsetSliderX->setSuffix(i18n(" px"));

    offsetSliderY->setSuffix(i18n(" px"));


    QStringList texturingModes;

    cmbTexturingMode->addItem(i18n("Multiply Alpha"), KisTextureProperties::MULTIPLY);
    cmbTexturingMode->addItem(i18n("Subtract Alpha"), KisTextureProperties::SUBTRACT);

    if (flags & SupportsLightnessMode) {
        cmbTexturingMode->addItem(i18n("Lightness Map"), KisTextureProperties::LIGHTNESS);
    }

    if (flags & SupportsGradientMode) {
        cmbTexturingMode->addItem(i18n("Gradient Map"), KisTextureProperties::GRADIENT);
    }

    selectTexturingMode(KisTextureProperties::SUBTRACT);

    QStringList cutOffPolicies;
    cutOffPolicies << i18n("Cut Off Disabled") << i18n("Cut Off Brush") << i18n("Cut Off Pattern");
    cmbCutoffPolicy->addItems(cutOffPolicies);


    cutoffSlider->setMinimumSize(256, 30);
    cutoffSlider->enableGamma(false);
    cutoffSlider->setToolTip(i18n("When pattern texture values are outside the range specified"
                                  " by the slider, the cut-off policy will be applied."));

    chkInvert->setChecked(false);

}

KisTextureChooser::~KisTextureChooser()
{

}

bool KisTextureChooser::selectTexturingMode(KisTextureProperties::TexturingMode mode)
{
    int i = 0;
    for (; i < cmbTexturingMode->count(); i++) {
        if (cmbTexturingMode->itemData(i) == mode) {
            cmbTexturingMode->setCurrentIndex(i);
        }
    }

    return i < cmbTexturingMode->count();
}

KisTextureProperties::TexturingMode KisTextureChooser::texturingMode() const
{
    return KisTextureProperties::TexturingMode(cmbTexturingMode->currentData().value<int>());
}
