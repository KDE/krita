/*
 *  SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>

 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_texture_chooser.h"
#include "kis_texture_option.h"
#include <KoCompositeOpRegistry.h>

#include <KisGradientChooser.h>

KisTextureChooser::KisTextureChooser(KisBrushTextureFlags flags, QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    textureSelectorWidget->setGrayscalePreview(true);
    textureSelectorWidget->setCurrentItem(0);

    scaleSlider->setRange(0.0, 2.0, 2);
    scaleSlider->setSingleStep(0.01);
    scaleSlider->setValue(1.0);
    scaleSlider->addMultiplier(0.1);
    scaleSlider->addMultiplier(2);
    scaleSlider->addMultiplier(10);

    brightnessSlider->setRange(-1.0, 1.0, 2);
    brightnessSlider->setSingleStep(0.01);
    brightnessSlider->setValue(0.0);
    brightnessSlider->setToolTip(i18n("Makes texture lighter or darker"));

    contrastSlider->setRange(0.0, 2.0, 2);
    contrastSlider->setSingleStep(0.01);
    contrastSlider->setValue(1.0);

    neutralPointSlider->setRange(0.0, 1.0, 2);
    neutralPointSlider->setSingleStep(0.01);
    neutralPointSlider->setValue(0.5);
    neutralPointSlider->setToolTip(i18n("Set gray value to be considered neutral for lightness mode"));

    offsetSliderX->setSuffix(i18n(" px"));

    offsetSliderY->setSuffix(i18n(" px"));


    QVector<std::pair<QString, int>> texturingModes;

    texturingModes
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_MULT).name(), KisTextureProperties::MULTIPLY)
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_SUBTRACT).name(), KisTextureProperties::SUBTRACT);

    if (flags & SupportsLightnessMode) {
        texturingModes << std::make_pair(i18nc("Lightness Map blend mode for brush texture", "Lightness Map"), KisTextureProperties::LIGHTNESS);
    }

    if (flags & SupportsGradientMode) {
        texturingModes << std::make_pair(i18nc("Gradient Map blend mode for brush texture", "Gradient Map"), KisTextureProperties::GRADIENT);
    }

    texturingModes
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_DARKEN).name(), KisTextureProperties::DARKEN)
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_OVERLAY).name(), KisTextureProperties::OVERLAY)
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_DODGE).name(), KisTextureProperties::COLOR_DODGE)
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_BURN).name(), KisTextureProperties::COLOR_BURN)
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_LINEAR_DODGE).name(), KisTextureProperties::LINEAR_DODGE)
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_LINEAR_BURN).name(), KisTextureProperties::LINEAR_BURN)
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_HARD_MIX_PHOTOSHOP).name(), KisTextureProperties::HARD_MIX_PHOTOSHOP)
        << std::make_pair(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP).name(), KisTextureProperties::HARD_MIX_SOFTER_PHOTOSHOP)
        << std::make_pair(i18nc("Height blend mode for brush texture", "Height"), KisTextureProperties::HEIGHT)
        << std::make_pair(i18nc("Linear Height blend mode for brush texture", "Linear Height"), KisTextureProperties::LINEAR_HEIGHT)
        << std::make_pair(i18nc("Height (Photoshop) blend mode for brush texture", "Height (Photoshop)"), KisTextureProperties::HEIGHT_PHOTOSHOP)
        << std::make_pair(i18nc("Linear Height (Photoshop) blend mode for brush texture", "Linear Height (Photoshop)"), KisTextureProperties::LINEAR_HEIGHT_PHOTOSHOP);

    for (auto it = texturingModes.begin(); it != texturingModes.end(); ++it) {
        cmbTexturingMode->addItem(it->first, it->second);
    }

    cmbTexturingMode->setCurrentIndex(KisTextureProperties::SUBTRACT);

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
