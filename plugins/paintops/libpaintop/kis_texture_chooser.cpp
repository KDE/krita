/*
 *  Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>

 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_texture_chooser.h"
#include "kis_texture_option.h"

KisTextureChooser::KisTextureChooser(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    textureSelectorWidget->setGrayscalePreview(true);
    textureSelectorWidget->setCurrentItem(0, 0);

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

    offsetSliderX->setSuffix(i18n(" px"));

    offsetSliderY->setSuffix(i18n(" px"));


    QStringList texturingModes;
    texturingModes << i18n("Multiply") << i18n("Subtract");
    cmbTexturingMode->addItems(texturingModes);
    cmbTexturingMode->setCurrentIndex(KisTextureProperties::SUBTRACT);

    QStringList cutOffPolicies;
    cutOffPolicies << i18n("Cut Off Disabled") << i18n("Cut Off Brush") << i18n("Cut Off Pattern");
    cmbCutoffPolicy->addItems(cutOffPolicies);


    cutoffSlider->setMinimumSize(20, 30);

    cutoffSlider->enableGamma(false);
    cutoffSlider->setToolTip(i18n("When pattern texture values are outside the range specified"
                                  " by the slider, the cut-off policy will be applied."));

    chkInvert->setChecked(false);
}

KisTextureChooser::~KisTextureChooser()
{

}
