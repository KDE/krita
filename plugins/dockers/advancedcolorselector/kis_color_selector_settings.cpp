/*
 *  Copyright (C) 2010 Celarek Adam <kdedev at xibo dot at>
 *
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

#include "kis_color_selector_settings.h"
#include "ui_wdg_color_selector_settings.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QComboBox>

#include <kconfiggroup.h>


#include <kis_icon.h>
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"

#include "kis_color_selector_combo_box.h"
#include "kis_color_selector.h"
#include "kis_config.h"


KisColorSelectorSettings::KisColorSelectorSettings(QWidget *parent) :
    KisPreferenceSet(parent),
    ui(new Ui::KisColorSelectorSettings)
{
    ui->setupUi(this);

    resize(minimumSize());

    ui->colorSelectorConfiguration->setColorSpace(ui->colorSpace->currentColorSpace());
    ui->useDifferentColorSpaceCheckbox->setChecked(false);

    connect(ui->useDifferentColorSpaceCheckbox, SIGNAL(clicked(bool)), this, SLOT(useDifferentColorSpaceChecked(bool)));

    /* color docker selector drop down */
    ui->dockerColorSettingsComboBox->addItem(i18n("Advanced Color Selector"));
    //ui->dockerColorSettingsComboBox->addItem(i18n("Color Sliders"));
    ui->dockerColorSettingsComboBox->addItem(i18n("Color Hotkeys"));
    ui->dockerColorSettingsComboBox->setCurrentIndex(0); // start off seeing advanced color selector properties

    connect( ui->dockerColorSettingsComboBox, SIGNAL(currentIndexChanged(int)),this, SLOT(changedColorDocker(int)));
    changedColorDocker(0);


    /* advanced color docker options */
    ui->dockerResizeOptionsComboBox->addItem(i18n("Change to a Horizontal Layout"));
    ui->dockerResizeOptionsComboBox->addItem(i18n("Hide Shade Selector"));
    ui->dockerResizeOptionsComboBox->addItem(i18n("Do Nothing"));
    ui->dockerResizeOptionsComboBox->setCurrentIndex(0);


    ui->zoomSelectorOptionComboBox->addItem(i18n("When Pressing Middle Mouse Button"));
    ui->zoomSelectorOptionComboBox->addItem(i18n("On Mouse Over"));
    ui->zoomSelectorOptionComboBox->addItem(i18n("Never"));
    ui->zoomSelectorOptionComboBox->setCurrentIndex(0);


    ui->colorSelectorTypeComboBox->addItem(i18n("HSV"));
    ui->colorSelectorTypeComboBox->addItem(i18n("HSL"));
    ui->colorSelectorTypeComboBox->addItem(i18n("HSI"));
    ui->colorSelectorTypeComboBox->addItem(i18n("HSY'"));
    ui->colorSelectorTypeComboBox->setCurrentIndex(0);
    connect( ui->colorSelectorTypeComboBox, SIGNAL(currentIndexChanged(int)),this, SLOT(changedACSColorSelectorType(int)));
    changedACSColorSelectorType(0); // initialize everything to HSV at the start


    ui->ACSshadeSelectorMyPaintColorModelComboBox->addItem(i18n("HSV"));
    ui->ACSshadeSelectorMyPaintColorModelComboBox->addItem(i18n("HSL"));
    ui->ACSshadeSelectorMyPaintColorModelComboBox->addItem(i18n("HSI"));
    ui->ACSshadeSelectorMyPaintColorModelComboBox->addItem(i18n("HSY'"));
    ui->ACSshadeSelectorMyPaintColorModelComboBox->setCurrentIndex(0);


    ui->ACSShadeSelectorTypeComboBox->addItem(i18n("MyPaint"));
    ui->ACSShadeSelectorTypeComboBox->addItem(i18n("Minimal"));
    ui->ACSShadeSelectorTypeComboBox->addItem(i18n("Do Not Show"));
    ui->ACSShadeSelectorTypeComboBox->setCurrentIndex(0);
    changedACSShadeSelectorType(0); // show/hide UI elements for MyPaint settings
    connect( ui->ACSShadeSelectorTypeComboBox, SIGNAL(currentIndexChanged(int)),this, SLOT(changedACSShadeSelectorType(int)));


    ui->commonColorsAlignVertical->setChecked(true);
    ui->commonColorsAlignHorizontal->setChecked(true);
    connect( ui->commonColorsAlignHorizontal, SIGNAL(toggled(bool)), this, SLOT(changedACSColorAlignment(bool)));
    connect( ui->lastUsedColorsAlignHorizontal, SIGNAL(toggled(bool)), this, SLOT(changedACSLastUsedColorAlignment(bool)));

    changedACSColorAlignment(ui->commonColorsAlignHorizontal->isChecked());
    changedACSLastUsedColorAlignment(ui->lastUsedColorsAlignHorizontal->isChecked());


    connect(ui->colorSpace,                 SIGNAL(colorSpaceChanged(const KoColorSpace*)),
            ui->colorSelectorConfiguration, SLOT(setColorSpace(const KoColorSpace*)));


    connect(this, SIGNAL(hsxchanged(int)),
            ui->colorSelectorConfiguration, SLOT(setList(int)));

    connect(ui->minimalShadeSelectorLineCount,      SIGNAL(valueChanged(int)),
            ui->minimalShadeSelectorLineSettings,   SLOT(setLineCount(int)));

    connect(ui->minimalShadeSelectorLineSettings,   SIGNAL(lineCountChanged(int)),
            ui->minimalShadeSelectorLineCount,      SLOT(setValue(int)));

    connect(ui->minimalShadeSelectorAsGradient,     SIGNAL(toggled(bool)),
            ui->minimalShadeSelectorLineSettings,   SIGNAL(setGradient(bool)));

    connect(ui->minimalShadeSelectorAsColorPatches, SIGNAL(toggled(bool)),
            ui->minimalShadeSelectorLineSettings,   SIGNAL(setPatches(bool)));

    connect(ui->minimalShadeSelectorLineHeight,     SIGNAL(valueChanged(int)),
            ui->minimalShadeSelectorLineSettings,   SIGNAL(setLineHeight(int)));

    connect(ui->minimalShadeSelectorPatchesPerLine, SIGNAL(valueChanged(int)),
            ui->minimalShadeSelectorLineSettings,   SIGNAL(setPatchCount(int)));
}

KisColorSelectorSettings::~KisColorSelectorSettings()
{
    delete ui;
}

QString KisColorSelectorSettings::id()
{
    return QString("advancedColorSelector");
}

QString KisColorSelectorSettings::name()
{
    return header();
}

QString KisColorSelectorSettings::header()
{
    return QString(i18n("Color Selector Settings"));
}


QIcon KisColorSelectorSettings::icon()
{
    return KisIconUtils::loadIcon("extended_color_selector");
}


void KisColorSelectorSettings::savePreferences() const
{
    // write cfg
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    KConfigGroup hsxcfg =  KSharedConfig::openConfig()->group("hsxColorSlider");
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");

    //  advanced color selector
    cfg.writeEntry("onDockerResize", ui->dockerResizeOptionsComboBox->currentIndex());
    cfg.writeEntry("zoomSelectorOptions", ui->zoomSelectorOptionComboBox->currentIndex() );
    cfg.writeEntry("zoomSize", ui->popupSize->value());
    cfg.writeEntry("showColorSelector", ui->chkShowColorSelector->isChecked());

    bool useCustomColorSpace =  ui->useDifferentColorSpaceCheckbox->isChecked();
    const KoColorSpace* colorSpace = useCustomColorSpace ? ui->colorSpace->currentColorSpace() : 0;

    KisConfig kisconfig(false);
    kisconfig.setCustomColorSelectorColorSpace(colorSpace);

    //color patches
    cfg.writeEntry("lastUsedColorsShow", ui->lastUsedColorsShow->isChecked());
    cfg.writeEntry("lastUsedColorsAlignment", ui->lastUsedColorsAlignVertical->isChecked());
    cfg.writeEntry("lastUsedColorsScrolling", ui->lastUsedColorsAllowScrolling->isChecked());
    cfg.writeEntry("lastUsedColorsNumCols", ui->lastUsedColorsNumCols->value());
    cfg.writeEntry("lastUsedColorsNumRows", ui->lastUsedColorsNumRows->value());
    cfg.writeEntry("lastUsedColorsCount", ui->lastUsedColorsPatchCount->value());
    cfg.writeEntry("lastUsedColorsWidth", ui->lastUsedColorsWidth->value());
    cfg.writeEntry("lastUsedColorsHeight", ui->lastUsedColorsHeight->value());

    cfg.writeEntry("commonColorsShow", ui->commonColorsShow->isChecked());
    cfg.writeEntry("commonColorsAlignment", ui->commonColorsAlignVertical->isChecked());
    cfg.writeEntry("commonColorsScrolling", ui->commonColorsAllowScrolling->isChecked());
    cfg.writeEntry("commonColorsNumCols", ui->commonColorsNumCols->value());
    cfg.writeEntry("commonColorsNumRows", ui->commonColorsNumRows->value());
    cfg.writeEntry("commonColorsCount", ui->commonColorsPatchCount->value());
    cfg.writeEntry("commonColorsWidth", ui->commonColorsWidth->value());
    cfg.writeEntry("commonColorsHeight", ui->commonColorsHeight->value());
    cfg.writeEntry("commonColorsAutoUpdate", ui->commonColorsAutoUpdate->isChecked());

    //shade selector



    int shadeSelectorTypeIndex =  ui->ACSShadeSelectorTypeComboBox->currentIndex();

    if(shadeSelectorTypeIndex == 0) {
        cfg.writeEntry("shadeSelectorType", "MyPaint");
    } else if (shadeSelectorTypeIndex == 1) {
        cfg.writeEntry("shadeSelectorType", "Minimal");
    } else {
        cfg.writeEntry("shadeSelectorType", "Hidden");
    }

    cfg.writeEntry("shadeSelectorUpdateOnRightClick", ui->shadeSelectorUpdateOnRightClick->isChecked());
    cfg.writeEntry("shadeSelectorUpdateOnForeground", ui->shadeSelectorUpdateOnForeground->isChecked());
    cfg.writeEntry("shadeSelectorUpdateOnLeftClick", ui->shadeSelectorUpdateOnLeftClick->isChecked());
    cfg.writeEntry("shadeSelectorUpdateOnBackground", ui->shadeSelectorUpdateOnBackground->isChecked());
    cfg.writeEntry("hidePopupOnClickCheck", ui->hidePopupOnClickCheck->isChecked());

    //mypaint model

    int shadeMyPaintComboBoxIndex  = ui->ACSshadeSelectorMyPaintColorModelComboBox->currentIndex();
    if (shadeMyPaintComboBoxIndex == 0 ) {
        cfg.writeEntry("shadeMyPaintType",   "HSV");
    } else  if (shadeMyPaintComboBoxIndex == 1 ) {
        cfg.writeEntry("shadeMyPaintType",   "HSL");
    } else  if (shadeMyPaintComboBoxIndex == 2 ) {
        cfg.writeEntry("shadeMyPaintType",   "HSI");
    } else {   // HSY
        cfg.writeEntry("shadeMyPaintType",   "HSY");
    }



    cfg.writeEntry("minimalShadeSelectorAsGradient", ui->minimalShadeSelectorAsGradient->isChecked());
    cfg.writeEntry("minimalShadeSelectorPatchCount", ui->minimalShadeSelectorPatchesPerLine->value());
    cfg.writeEntry("minimalShadeSelectorLineConfig",  ui->minimalShadeSelectorLineSettings->toString());
    cfg.writeEntry("minimalShadeSelectorLineHeight", ui->minimalShadeSelectorLineHeight->value());

    //color selector
    KisColorSelectorComboBox* cstw = dynamic_cast<KisColorSelectorComboBox*>(ui->colorSelectorConfiguration);
    cfg.writeEntry("colorSelectorConfiguration", cstw->configuration().toString());

    cfg.writeEntry("hsxSettingType", ui->colorSelectorTypeComboBox->currentIndex());

    //luma//
    cfg.writeEntry("lumaR", ui->l_lumaR->value());
    cfg.writeEntry("lumaG", ui->l_lumaG->value());
    cfg.writeEntry("lumaB", ui->l_lumaB->value());
    cfg.writeEntry("gamma", ui->SP_Gamma->value());

    //slider//
    hsxcfg.writeEntry("hsvH", ui->csl_hsvH->isChecked());
    hsxcfg.writeEntry("hsvS", ui->csl_hsvS->isChecked());
    hsxcfg.writeEntry("hsvV", ui->csl_hsvV->isChecked());
    hsxcfg.writeEntry("hslH", ui->csl_hslH->isChecked());
    hsxcfg.writeEntry("hslS", ui->csl_hslS->isChecked());
    hsxcfg.writeEntry("hslL", ui->csl_hslL->isChecked());
    hsxcfg.writeEntry("hsiH", ui->csl_hsiH->isChecked());
    hsxcfg.writeEntry("hsiS", ui->csl_hsiS->isChecked());
    hsxcfg.writeEntry("hsiI", ui->csl_hsiI->isChecked());
    hsxcfg.writeEntry("hsyH", ui->csl_hsyH->isChecked());
    hsxcfg.writeEntry("hsyS", ui->csl_hsyS->isChecked());
    hsxcfg.writeEntry("hsyY", ui->csl_hsyY->isChecked());

    //hotkeys//
    hotkeycfg.writeEntry("steps_lightness", ui->sb_lightness->value());
    hotkeycfg.writeEntry("steps_saturation", ui->sb_saturation->value());
    hotkeycfg.writeEntry("steps_hue", ui->sb_hue->value());
    hotkeycfg.writeEntry("steps_redgreen", ui->sb_rg->value());
    hotkeycfg.writeEntry("steps_blueyellow", ui->sb_by->value());

    emit settingsChanged();
}

//void KisColorSelectorSettings::changeEvent(QEvent *e)
//{
//    QDialog::changeEvent(e);
//    switch (e->type()) {
//    case QEvent::LanguageChange:
//        ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
//}


void KisColorSelectorSettings::changedColorDocker(int index)
{
    // having a situation where too many sections are visible makes the window too large. turn all off before turning more on
    ui->colorSliderOptions->hide();
    ui->advancedColorSelectorOptions->hide();
    ui->hotKeyOptions->hide();

    if (index == 0)     { // advanced color selector options selected
        ui->advancedColorSelectorOptions->show();
        ui->colorSliderOptions->hide();
        ui->hotKeyOptions->hide();
    }
//    else if (index == 1) {  // color slider options selected
//        ui->advancedColorSelectorOptions->hide();
//        ui->hotKeyOptions->hide();
//        ui->colorSliderOptions->show();
//    }
    else {
       ui->colorSliderOptions->hide();
       ui->advancedColorSelectorOptions->hide();
       ui->hotKeyOptions->show();
    }
}

void KisColorSelectorSettings::changedACSColorSelectorType(int index)
{
    ui->lumaCoefficientGroupbox->setVisible(false);

    if (index == 0)     {  // HSV
        ui->ACSTypeDescriptionLabel->setText(i18n("Values goes from black to white, or black to the most saturated colour. Saturation, in turn, goes from the most saturated colour to white, grey or black."));
    }
    else if (index == 1)     {  // HSL
        ui->ACSTypeDescriptionLabel->setText(i18n("Lightness goes from black to white, with middle grey being equal to the most saturated colour."));
    }
    else if (index == 2)     {  // HSI
        ui->ACSTypeDescriptionLabel->setText(i18n("Intensity maps to the sum of rgb components"));
    }
    else {  // HSY'
        ui->ACSTypeDescriptionLabel->setText(i18n("Luma(Y') is weighted by its coefficients which are configurable. Default values are set to 'rec 709'."));
        ui->lumaCoefficientGroupbox->setVisible(true);
    }

    ui->colorSelectorConfiguration->update();
    emit hsxchanged(index);

}


void KisColorSelectorSettings::changedACSColorAlignment(bool toggled)
{
    // this slot is tied to the horizontal radio button's state being changed
    // you can infer the vertical state

    ui->lbl_commonColorsNumCols->setDisabled(toggled);
    ui->commonColorsNumCols->setDisabled(toggled);

    ui->lbl_commonColorsNumRows->setEnabled(toggled);
    ui->commonColorsNumRows->setEnabled(toggled);
}

void KisColorSelectorSettings::changedACSLastUsedColorAlignment(bool toggled)
{
    // this slot is tied to the horizontal radio button's state being changed
    // you can infer the vertical state

    ui->lbl_lastUsedNumCols->setDisabled(toggled);
    ui->lastUsedColorsNumCols->setDisabled(toggled);

    ui->lbl_lastUsedNumRows->setEnabled(toggled);
    ui->lastUsedColorsNumRows->setEnabled(toggled);
}

void KisColorSelectorSettings::changedACSShadeSelectorType(int index)
{

    if (index == 0)     {  // MyPaint
        ui->minimalShadeSelectorGroup->hide();
        ui->myPaintColorModelLabel->show();
        ui->ACSshadeSelectorMyPaintColorModelComboBox->show();

    } else if (index == 1) { // Minimal
        ui->minimalShadeSelectorGroup->show();
        ui->myPaintColorModelLabel->hide();
        ui->ACSshadeSelectorMyPaintColorModelComboBox->hide();

    }else { // do not show
        ui->minimalShadeSelectorGroup->hide();
        ui->myPaintColorModelLabel->hide();
        ui->ACSshadeSelectorMyPaintColorModelComboBox->hide();
    }
}

void KisColorSelectorSettings::useDifferentColorSpaceChecked(bool enabled)
{
    ui->colorSpace->setEnabled(enabled);
}

void KisColorSelectorSettings::loadPreferences()
{
    //read cfg
    //don't forget to also add a new entry to the default preferences

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    KConfigGroup hsxcfg =  KSharedConfig::openConfig()->group("hsxColorSlider");
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");


    // Advanced color selector
    ui->dockerResizeOptionsComboBox->setCurrentIndex(  (int)cfg.readEntry("onDockerResize", 0) );
    ui->zoomSelectorOptionComboBox->setCurrentIndex(   (int) cfg.readEntry("zoomSelectorOptions", 0) );
    ui->popupSize->setValue(cfg.readEntry("zoomSize", 280));
    ui->chkShowColorSelector->setChecked((bool) cfg.readEntry("showColorSelector", true));

    {
        KisConfig kisconfig(true);
        const KoColorSpace *cs = kisconfig.customColorSelectorColorSpace();

        if (cs) {
            ui->useDifferentColorSpaceCheckbox->setChecked(true);
            ui->colorSpace->setEnabled(true);
            ui->colorSpace->setCurrentColorSpace(cs);
        } else {
            ui->useDifferentColorSpaceCheckbox->setChecked(false);
            ui->colorSpace->setEnabled(false);
        }
    }


    //color patches
    ui->lastUsedColorsShow->setChecked(cfg.readEntry("lastUsedColorsShow", true));
    bool a = cfg.readEntry("lastUsedColorsAlignment", true);
    ui->lastUsedColorsAlignVertical->setChecked(a);
    ui->lastUsedColorsAlignHorizontal->setChecked(!a);
    ui->lastUsedColorsAllowScrolling->setChecked(cfg.readEntry("lastUsedColorsScrolling", true));
    ui->lastUsedColorsNumCols->setValue(cfg.readEntry("lastUsedColorsNumCols", 1));
    ui->lastUsedColorsNumRows->setValue(cfg.readEntry("lastUsedColorsNumRows", 1));
    ui->lastUsedColorsPatchCount->setValue(cfg.readEntry("lastUsedColorsCount", 20));
    ui->lastUsedColorsWidth->setValue(cfg.readEntry("lastUsedColorsWidth", 16));
    ui->lastUsedColorsHeight->setValue(cfg.readEntry("lastUsedColorsHeight", 16));

    ui->commonColorsShow->setChecked(cfg.readEntry("commonColorsShow", true));
    a = cfg.readEntry("commonColorsAlignment", false);
    ui->commonColorsAlignVertical->setChecked(a);
    ui->commonColorsAlignHorizontal->setChecked(!a);
    ui->commonColorsAllowScrolling->setChecked(cfg.readEntry("commonColorsScrolling", true));
    ui->commonColorsNumCols->setValue(cfg.readEntry("commonColorsNumCols", 1));
    ui->commonColorsNumRows->setValue(cfg.readEntry("commonColorsNumRows", 1));
    ui->commonColorsPatchCount->setValue(cfg.readEntry("commonColorsCount", 12));
    ui->commonColorsWidth->setValue(cfg.readEntry("commonColorsWidth", 16));
    ui->commonColorsHeight->setValue(cfg.readEntry("commonColorsHeight", 16));
    ui->commonColorsAutoUpdate->setChecked(cfg.readEntry("commonColorsAutoUpdate", false));

    //shade selector
    QString shadeSelectorType=cfg.readEntry("shadeSelectorType", "Minimal");

    if ( shadeSelectorType == "MyPaint") {
        ui->ACSShadeSelectorTypeComboBox->setCurrentIndex(0);
    } else if (shadeSelectorType == "Minimal") {
        ui->ACSShadeSelectorTypeComboBox->setCurrentIndex(1);
    } else {   // Hidden
        ui->ACSShadeSelectorTypeComboBox->setCurrentIndex(2);
    }

    ui->shadeSelectorUpdateOnRightClick->setChecked(cfg.readEntry("shadeSelectorUpdateOnRightClick", false));
    ui->shadeSelectorUpdateOnLeftClick->setChecked(cfg.readEntry("shadeSelectorUpdateOnLeftClick", false));
    ui->shadeSelectorUpdateOnForeground->setChecked(cfg.readEntry("shadeSelectorUpdateOnForeground", true));
    ui->shadeSelectorUpdateOnBackground->setChecked(cfg.readEntry("shadeSelectorUpdateOnBackground", true));
    ui->hidePopupOnClickCheck->setChecked(cfg.readEntry("hidePopupOnClickCheck", false));

    QString shadeMyPaintType = cfg.readEntry("shadeMyPaintType", "HSV");

    if (shadeMyPaintType == "HSV" ) {
        ui->ACSshadeSelectorMyPaintColorModelComboBox->setCurrentIndex(0);
    } else  if (shadeMyPaintType == "HSL" ) {
        ui->ACSshadeSelectorMyPaintColorModelComboBox->setCurrentIndex(1);
    } else  if (shadeMyPaintType == "HSI" ) {
        ui->ACSshadeSelectorMyPaintColorModelComboBox->setCurrentIndex(2);
    } else {   // HSY
        ui->ACSshadeSelectorMyPaintColorModelComboBox->setCurrentIndex(3);
    }


    bool asGradient = cfg.readEntry("minimalShadeSelectorAsGradient", true);
    if(asGradient) ui->minimalShadeSelectorAsGradient->setChecked(true);
    else ui->minimalShadeSelectorAsColorPatches->setChecked(true);

    ui->minimalShadeSelectorPatchesPerLine->setValue(cfg.readEntry("minimalShadeSelectorPatchCount", 10));
    ui->minimalShadeSelectorLineSettings->fromString(cfg.readEntry("minimalShadeSelectorLineConfig", "0|0.2|0|0|0|0|0;1|0|1|1|0|0|0;2|0|-1|1|0|0|0;"));
    ui->minimalShadeSelectorLineHeight->setValue(cfg.readEntry("minimalShadeSelectorLineHeight", 10));

    int hsxSettingType= (int)cfg.readEntry("hsxSettingType", 0);
    ui->colorSelectorTypeComboBox->setCurrentIndex(hsxSettingType);


    //color selector
    KisColorSelectorComboBox* cstw = dynamic_cast<KisColorSelectorComboBox*>(ui->colorSelectorConfiguration);
    cstw->setConfiguration(KisColorSelectorConfiguration::fromString(cfg.readEntry("colorSelectorConfiguration", "3|0|5|0"))); // triangle selector

    //luma values//
    ui->l_lumaR->setValue(cfg.readEntry("lumaR", 0.2126));
    ui->l_lumaG->setValue(cfg.readEntry("lumaG", 0.7152));
    ui->l_lumaB->setValue(cfg.readEntry("lumaB", 0.0722));
    ui->SP_Gamma->setValue(cfg.readEntry("gamma", 2.2));

    //color sliders//
    ui->csl_hsvH->setChecked(hsxcfg.readEntry("hsvH", false));
    ui->csl_hsvS->setChecked(hsxcfg.readEntry("hsvS", false));
    ui->csl_hsvV->setChecked(hsxcfg.readEntry("hsvV", false));
    ui->csl_hslH->setChecked(hsxcfg.readEntry("hslH", true));
    ui->csl_hslS->setChecked(hsxcfg.readEntry("hslS", true));
    ui->csl_hslL->setChecked(hsxcfg.readEntry("hslL", true));
    ui->csl_hsiH->setChecked(hsxcfg.readEntry("hsiH", false));
    ui->csl_hsiS->setChecked(hsxcfg.readEntry("hsiS", false));
    ui->csl_hsiI->setChecked(hsxcfg.readEntry("hsiI", false));
    ui->csl_hsyH->setChecked(hsxcfg.readEntry("hsyH", false));
    ui->csl_hsyS->setChecked(hsxcfg.readEntry("hsyS", false));
    ui->csl_hsyY->setChecked(hsxcfg.readEntry("hsyY", false));

    //hotkeys//
    ui->sb_lightness->setValue(hotkeycfg.readEntry("steps_lightness", 10));
    ui->sb_saturation->setValue(hotkeycfg.readEntry("steps_saturation", 10));
    ui->sb_hue->setValue(hotkeycfg.readEntry("steps_hue", 36));
    ui->sb_rg->setValue(hotkeycfg.readEntry("steps_redgreen", 10));
    ui->sb_by->setValue(hotkeycfg.readEntry("steps_blueyellow", 10));


}

void KisColorSelectorSettings::loadDefaultPreferences()
{
    //set defaults
    //if you change something, don't forget that loadPreferences should be kept in sync

    // advanced color selector docker
    ui->dockerResizeOptionsComboBox->setCurrentIndex(0);
    ui->zoomSelectorOptionComboBox->setCurrentIndex(0);
    ui->popupSize->setValue(280);
    ui->chkShowColorSelector->setChecked(true);


    ui->useDifferentColorSpaceCheckbox->setChecked(false);
    ui->colorSpace->setCurrentColorModel(KoID("RGBA"));
    ui->colorSpace->setCurrentColorDepth(KoID("U8"));
    ui->colorSpace->setCurrentProfile(KoColorSpaceRegistry::instance()->rgb8()->profile()->name());

    //color patches
    ui->lastUsedColorsShow->setChecked(true);
    ui->lastUsedColorsAlignVertical->setChecked(true);
    ui->lastUsedColorsAlignHorizontal->setChecked(false);
    ui->lastUsedColorsAllowScrolling->setChecked(true);
    ui->lastUsedColorsNumCols->setValue(1);
    ui->lastUsedColorsNumRows->setValue(1);
    ui->lastUsedColorsPatchCount->setValue(20);
    ui->lastUsedColorsWidth->setValue(16);
    ui->lastUsedColorsHeight->setValue(16);

    ui->commonColorsShow->setChecked(true);
    ui->commonColorsAlignVertical->setChecked(false);
    ui->commonColorsAlignHorizontal->setChecked(true);
    ui->commonColorsAllowScrolling->setChecked(true);
    ui->commonColorsNumCols->setValue(1);
    ui->commonColorsNumRows->setValue(1);
    ui->commonColorsPatchCount->setValue(12);
    ui->commonColorsWidth->setValue(16);
    ui->commonColorsHeight->setValue(16);
    ui->commonColorsAutoUpdate->setChecked(false);

    //shade selector
    ui->ACSShadeSelectorTypeComboBox->setCurrentIndex(1); // Minimal

    ui->ACSshadeSelectorMyPaintColorModelComboBox->setCurrentIndex(0);

    ui->shadeSelectorUpdateOnRightClick->setChecked(false);
    ui->shadeSelectorUpdateOnLeftClick->setChecked(false);
    ui->shadeSelectorUpdateOnForeground->setChecked(true);
    ui->shadeSelectorUpdateOnBackground->setChecked(true);

    bool asGradient = true;
    if(asGradient) ui->minimalShadeSelectorAsGradient->setChecked(true);
    else ui->minimalShadeSelectorAsColorPatches->setChecked(true);

    ui->minimalShadeSelectorPatchesPerLine->setValue(10);
    ui->minimalShadeSelectorLineSettings->fromString("0|0.2|0|0|0|0|0;1|0|1|1|0|0|0;2|0|-1|1|0|0|0;");
    ui->minimalShadeSelectorLineHeight->setValue(10);

    // set advanced color selector to use HSV
    ui->colorSelectorTypeComboBox->setCurrentIndex(0);

    KisColorSelectorComboBox* cstw = dynamic_cast<KisColorSelectorComboBox*>(ui->colorSelectorConfiguration);
    cstw->setConfiguration(KisColorSelectorConfiguration("3|0|5|0")); // triangle selector

    //luma//
    ui->l_lumaR->setValue(0.2126);
    ui->l_lumaG->setValue(0.7152);
    ui->l_lumaB->setValue(0.0722);
    ui->SP_Gamma->setValue(2.2);

    //color sliders//
    ui->csl_hsvH->setChecked(false);
    ui->csl_hsvS->setChecked(false);
    ui->csl_hsvV->setChecked(false);
    ui->csl_hslH->setChecked(true);
    ui->csl_hslS->setChecked(true);
    ui->csl_hslL->setChecked(true);
    ui->csl_hsiH->setChecked(false);
    ui->csl_hsiS->setChecked(false);
    ui->csl_hsiI->setChecked(false);
    ui->csl_hsyH->setChecked(false);
    ui->csl_hsyS->setChecked(false);
    ui->csl_hsyY->setChecked(false);

    //hotkeys//
    ui->sb_lightness->setValue(10);
    ui->sb_saturation->setValue(10);
    ui->sb_hue->setValue(36);
    ui->sb_rg->setValue(10);
    ui->sb_by->setValue(10);

}

KisColorSelectorSettingsDialog::KisColorSelectorSettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_widget(new KisColorSelectorSettings(this))
{
    QLayout* l = new QVBoxLayout(this);
    l->addWidget(m_widget);

    m_widget->loadPreferences();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::RestoreDefaults,
                                                       Qt::Horizontal,
                                                       this);
    l->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), m_widget, SLOT(savePreferences()));
    connect(buttonBox, SIGNAL(accepted()), this,     SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this,     SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults),
            SIGNAL(clicked()),  m_widget, SLOT(loadDefaultPreferences()));
}



