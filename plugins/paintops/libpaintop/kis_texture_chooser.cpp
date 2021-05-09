/*
 *  SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>

 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_texture_chooser.h"
#include "kis_texture_option.h"
#include <KoCompositeOpRegistry.h>

#include "widgets/kis_gradient_chooser.h"

KisTextureChooser::KisTextureChooser(QWidget *parent)
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
    texturingModes
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_MULT).name()
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_SUBTRACT).name()
        << i18nc("Lightness Map blend mode for brush texture", "Lightness Map")
        << i18nc("Gradient Map blend mode for brush texture", "Gradient Map")
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_DARKEN).name()
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_OVERLAY).name()
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_DODGE).name()
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_BURN).name()
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_LINEAR_DODGE).name()
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_LINEAR_BURN).name()
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_HARD_MIX_PHOTOSHOP).name()
        << KoCompositeOpRegistry::instance().getKoID(COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP).name()
        << i18nc("Height blend mode for brush texture", "Height")
        << i18nc("Linear Height blend mode for brush texture", "Linear Height")
        << i18nc("Height (Photoshop) blend mode for brush texture", "Height (Photoshop)")
        << i18nc("Linear Height (Photoshop) blend mode for brush texture", "Linear Height (Photoshop)");
    cmbTexturingMode->addItems(texturingModes);
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
