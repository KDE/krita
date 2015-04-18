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

#include <kconfiggroup.h>

#include <KoIcon.h>
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
    ui->lbl_lastUsedNumRows->hide();
    ui->lastUsedColorsNumRows->hide();

    ui->lbl_commonColorsNumCols->hide();
    ui->commonColorsNumCols->hide();
    
    if (ui->colorSelectorHSVtype->isChecked()==false){
        ui->l_HSVtypeInfo->hide();
    }
    ui->l_HSLtypeInfo->hide();
    ui->l_HSItypeInfo->hide();
    ui->l_HSYtypeInfo->hide();
    resize(minimumSize());

    ui->colorSelectorConfiguration->setColorSpace(ui->colorSpace->currentColorSpace());

    connect(ui->colorSpace,                 SIGNAL(colorSpaceChanged(const KoColorSpace*)),
            ui->colorSelectorConfiguration, SLOT(setColorSpace(const KoColorSpace*)));
    connect(ui->colorSelectorHSVtype, SIGNAL(toggled(bool)),
            this, SLOT(hsxchange()));
    connect(ui->colorSelectorHSLtype, SIGNAL(toggled(bool)),
            this, SLOT(hsxchange()));
    connect(ui->colorSelectorHSItype, SIGNAL(toggled(bool)),
            this, SLOT(hsxchange()));
    connect(ui->colorSelectorHSYtype, SIGNAL(toggled(bool)),
            this, SLOT(hsxchange()));

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


KIcon KisColorSelectorSettings::icon()
{
    return koIcon("extended_color_selector");
}


void KisColorSelectorSettings::savePreferences() const
{
    //write cfg
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    KConfigGroup hsxcfg = KGlobal::config()->group("hsxColorSlider");

    //general
    cfg.writeEntry("shadeSelectorHideable", ui->shadeSelectorHideable->isChecked());
    cfg.writeEntry("allowHorizontalLayout", ui->allowHorizontalLayout->isChecked());
    cfg.writeEntry("popupOnMouseOver", ui->popupOnMouseOver->isChecked());
    cfg.writeEntry("popupOnMouseClick", ui->popupOnMouseClick->isChecked());
    cfg.writeEntry("zoomSize", ui->popupSize->value());

    bool useCustomColorSpace = ui->useCustomColorSpace->isChecked();
    const KoColorSpace* colorSpace = useCustomColorSpace ?
                ui->colorSpace->currentColorSpace() : 0;

    KisConfig kisconfig;
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
    QString shadeSelectorType("MyPaint");
    if(ui->shadeSelectorTypeMinimal->isChecked())
        shadeSelectorType="Minimal";
    if(ui->shadeSelectorTypeHidden->isChecked())
        shadeSelectorType="Hidden";

    cfg.writeEntry("shadeSelectorType", shadeSelectorType);

    cfg.writeEntry("shadeSelectorUpdateOnRightClick", ui->shadeSelectorUpdateOnRightClick->isChecked());
    cfg.writeEntry("shadeSelectorUpdateOnForeground", ui->shadeSelectorUpdateOnForeground->isChecked());
    cfg.writeEntry("shadeSelectorUpdateOnLeftClick", ui->shadeSelectorUpdateOnLeftClick->isChecked());
    cfg.writeEntry("shadeSelectorUpdateOnBackground", ui->shadeSelectorUpdateOnBackground->isChecked());

    //mypaint model
    QString shadeMyPaintType("HSV");
    if(ui->MyPaint_HSL->isChecked())
        shadeMyPaintType="HSL";
    if(ui->MyPaint_HSI->isChecked())
        shadeMyPaintType="HSI";
    if(ui->MyPaint_HSY->isChecked())
        shadeMyPaintType="HSY";

    cfg.writeEntry("shadeMyPaintType", shadeMyPaintType);

    cfg.writeEntry("minimalShadeSelectorAsGradient", ui->minimalShadeSelectorAsGradient->isChecked());
    cfg.writeEntry("minimalShadeSelectorPatchCount", ui->minimalShadeSelectorPatchesPerLine->value());
    cfg.writeEntry("minimalShadeSelectorLineConfig",  ui->minimalShadeSelectorLineSettings->toString());
    cfg.writeEntry("minimalShadeSelectorLineHeight", ui->minimalShadeSelectorLineHeight->value());

    //color selector
    cfg.writeEntry("hideColorSelector", ui->hideColorSelector->isChecked());

    KisColorSelectorComboBox* cstw = dynamic_cast<KisColorSelectorComboBox*>(ui->colorSelectorConfiguration);
    cfg.writeEntry("colorSelectorConfiguration", cstw->configuration().toString());
    
    QString hsxSettingType("HSV");
    if(ui->colorSelectorHSLtype->isChecked())
        hsxSettingType="HSL";
    if(ui->colorSelectorHSItype->isChecked())
        hsxSettingType="HSI";
    if(ui->colorSelectorHSYtype->isChecked())
        hsxSettingType="HSY";
    
    cfg.writeEntry("hsxSettingType", hsxSettingType);
    
    //luma//
    cfg.writeEntry("lumaR", ui->l_lumaR->text());
    cfg.writeEntry("lumaG", ui->l_lumaG->text());
    cfg.writeEntry("lumaB", ui->l_lumaB->text());
    
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

void KisColorSelectorSettings::loadPreferences()
{
    //read cfg
    //don't forget to also add a new entry to the default preferences

    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    KConfigGroup hsxcfg = KGlobal::config()->group("hsxColorSlider");

    //general

    //it's not possible to set a radio box to false. additionally, we need to set shrunkenDonothing to true, in case..
    bool a = cfg.readEntry("shadeSelectorHideable", false);
    bool b = cfg.readEntry("allowHorizontalLayout", true);
    if(a)
        ui->shadeSelectorHideable->setChecked(true);
    else if(b)
        ui->allowHorizontalLayout->setChecked(true);
    else
        ui->shrunkenDoNothing->setChecked(true);


    {
        KisConfig kisconfig;
        const KoColorSpace *cs = kisconfig.customColorSelectorColorSpace();

        if(cs) {
            ui->useCustomColorSpace->setChecked(true);
            ui->colorSpace->setCurrentColorSpace(cs);
        } else {
            ui->useImageColorSpace->setChecked(true);
        }
    }

    a = cfg.readEntry("popupOnMouseOver", false);
    b = cfg.readEntry("popupOnMouseClick", true);
    if(a)
        ui->popupOnMouseOver->setChecked(true);
    else if(b)
        ui->popupOnMouseClick->setChecked(true);
    else
        ui->neverZoom->setChecked(true);

    ui->popupSize->setValue(cfg.readEntry("zoomSize", 280));

    //color patches
    ui->lastUsedColorsShow->setChecked(cfg.readEntry("lastUsedColorsShow", true));
    a = cfg.readEntry("lastUsedColorsAlignment", true);
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
    ui->commonColorsAllowScrolling->setChecked(cfg.readEntry("commonColorsScrolling", false));
    ui->commonColorsNumCols->setValue(cfg.readEntry("commonColorsNumCols", 1));
    ui->commonColorsNumRows->setValue(cfg.readEntry("commonColorsNumRows", 1));
    ui->commonColorsPatchCount->setValue(cfg.readEntry("commonColorsCount", 12));
    ui->commonColorsWidth->setValue(cfg.readEntry("commonColorsWidth", 16));
    ui->commonColorsHeight->setValue(cfg.readEntry("commonColorsHeight", 16));
    ui->commonColorsAutoUpdate->setChecked(cfg.readEntry("commonColorsAutoUpdate", false));

    //shade selector
    QString shadeSelectorType=cfg.readEntry("shadeSelectorType", "MyPaint");
    ui->shadeSelectorTypeMyPaint->setChecked(shadeSelectorType=="MyPaint");
    ui->shadeSelectorTypeMinimal->setChecked(shadeSelectorType=="Minimal");
    ui->shadeSelectorTypeHidden->setChecked(shadeSelectorType=="Hidden");

    ui->shadeSelectorUpdateOnRightClick->setChecked(cfg.readEntry("shadeSelectorUpdateOnRightClick", false));
    ui->shadeSelectorUpdateOnLeftClick->setChecked(cfg.readEntry("shadeSelectorUpdateOnLeftClick", false));
    ui->shadeSelectorUpdateOnForeground->setChecked(cfg.readEntry("shadeSelectorUpdateOnForeground", true));
    ui->shadeSelectorUpdateOnBackground->setChecked(cfg.readEntry("shadeSelectorUpdateOnBackground", true));

    QString shadeMyPaintType=cfg.readEntry("shadeMyPaintType", "HSV");
    ui->MyPaint_HSV->setChecked(shadeMyPaintType=="HSV");
    ui->MyPaint_HSL->setChecked(shadeMyPaintType=="HSL");
    ui->MyPaint_HSI->setChecked(shadeMyPaintType=="HSI");
    ui->MyPaint_HSY->setChecked(shadeMyPaintType=="HSY");

    bool asGradient = cfg.readEntry("minimalShadeSelectorAsGradient", true);
    if(asGradient) ui->minimalShadeSelectorAsGradient->setChecked(true);
    else ui->minimalShadeSelectorAsColorPatches->setChecked(true);

    ui->minimalShadeSelectorPatchesPerLine->setValue(cfg.readEntry("minimalShadeSelectorPatchCount", 10));
    ui->minimalShadeSelectorLineSettings->fromString(cfg.readEntry("minimalShadeSelectorLineConfig", "0|0.2|0|0|0|0|0;1|0|1|1|0|0|0;2|0|-1|1|0|0|0;"));
    ui->minimalShadeSelectorLineHeight->setValue(cfg.readEntry("minimalShadeSelectorLineHeight", 10));

    ui->hideColorSelector->setChecked(cfg.readEntry("hideColorSelector", true));

    QString hsxSettingType=cfg.readEntry("hsxSettingType", "HSV");
    ui->colorSelectorHSVtype->setChecked(hsxSettingType=="HSV");
    ui->colorSelectorHSLtype->setChecked(hsxSettingType=="HSL");
    ui->colorSelectorHSItype->setChecked(hsxSettingType=="HSI");
    ui->colorSelectorHSYtype->setChecked(hsxSettingType=="HSY");
    //color selector
    KisColorSelectorComboBox* cstw = dynamic_cast<KisColorSelectorComboBox*>(ui->colorSelectorConfiguration);
    cstw->setConfiguration(KisColorSelector::Configuration::fromString(cfg.readEntry("colorSelectorConfiguration", "3|0|5|0"))); // triangle selector
    
    //luma values//
    ui->l_lumaR->setText(cfg.readEntry("lumaR", "0.2126"));
    ui->l_lumaG->setText(cfg.readEntry("lumaG", "0.7152"));
    ui->l_lumaB->setText(cfg.readEntry("lumaB", "0.0722"));
    
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
    
}

void KisColorSelectorSettings::loadDefaultPreferences()
{
    //set defaults
    //if you change something, don't forget that loadPreferences should be kept in sync

    //general
    ui->allowHorizontalLayout->setChecked(false);
    ui->shadeSelectorHideable->setChecked(false);
    ui->allowHorizontalLayout->setChecked(true);

    ui->popupOnMouseClick->setChecked(true);
    ui->popupOnMouseOver->setChecked(false);
    ui->neverZoom->setChecked(false);
    ui->popupSize->setValue(280);

    ui->useImageColorSpace->setChecked(true);
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
    ui->commonColorsAllowScrolling->setChecked(false);
    ui->commonColorsNumCols->setValue(1);
    ui->commonColorsNumRows->setValue(1);
    ui->commonColorsPatchCount->setValue(12);
    ui->commonColorsWidth->setValue(16);
    ui->commonColorsHeight->setValue(16);
    ui->commonColorsAutoUpdate->setChecked(false);

    //shade selector
    ui->shadeSelectorTypeMyPaint->setChecked(true);
    ui->shadeSelectorTypeMinimal->setChecked(false);
    ui->shadeSelectorTypeHidden->setChecked(false);

    ui->MyPaint_HSV->setChecked(true);
    ui->MyPaint_HSL->setChecked(false);
    ui->MyPaint_HSI->setChecked(false);
    ui->MyPaint_HSY->setChecked(false);

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

    //color selector
    ui->colorSelectorHSVtype->setChecked(true);
    ui->colorSelectorHSVtype->setChecked(false);
    ui->colorSelectorHSVtype->setChecked(false);
    ui->colorSelectorHSVtype->setChecked(false);

    ui->hideColorSelector->setChecked(false);
    
    KisColorSelectorComboBox* cstw = dynamic_cast<KisColorSelectorComboBox*>(ui->colorSelectorConfiguration);
    cstw->setConfiguration(KisColorSelector::Configuration("3|0|5|0")); // triangle selector
    
    //luma//
    ui->l_lumaR->setText("0.2126");
    ui->l_lumaG->setText("0.7152");
    ui->l_lumaB->setText("0.0722");
    
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
}

void KisColorSelectorSettings::hsxchange() {

    int hsxSettingType=0;
    if(ui->colorSelectorHSLtype->isChecked())
        hsxSettingType=1;
    if(ui->colorSelectorHSItype->isChecked())
        hsxSettingType=2;
    if(ui->colorSelectorHSYtype->isChecked())
        hsxSettingType=3;

    emit hsxchanged(hsxSettingType);
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



