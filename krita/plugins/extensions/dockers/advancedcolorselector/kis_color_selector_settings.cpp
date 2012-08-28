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

#include <KConfigGroup>

#include <KoIcon.h>
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"

#include "kis_color_selector_combo_box.h"
#include "kis_color_selector.h"

KisColorSelectorSettings::KisColorSelectorSettings(QWidget *parent) :
    KisPreferenceSet(parent),
    ui(new Ui::KisColorSelectorSettings)
{
    ui->setupUi(this);
    ui->lbl_lastUsedNumRows->hide();
    ui->lastUsedColorsNumRows->hide();

    ui->lbl_commonColorsNumCols->hide();
    ui->commonColorsNumCols->hide();
    resize(minimumSize());

    ui->colorSelectorConfiguration->setColorSpace(ui->colorSpace->currentColorSpace());

    connect(ui->colorSpace,                 SIGNAL(colorSpaceChanged(const KoColorSpace*)),
            ui->colorSelectorConfiguration, SLOT(setColorSpace(const KoColorSpace*)));

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
    return QString("Color Selector Settings");
}


KIcon KisColorSelectorSettings::icon()
{
    return koIcon("extended_color_selector");
}


void KisColorSelectorSettings::savePreferences() const
{
    //write cfg
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");

    //general
    cfg.writeEntry("shadeSelectorHideable", ui->shadeSelectorHideable->isChecked());
    cfg.writeEntry("allowHorizontalLayout", ui->allowHorizontalLayout->isChecked());
    cfg.writeEntry("popupOnMouseOver", ui->popupOnMouseOver->isChecked());
    cfg.writeEntry("popupOnMouseClick", ui->popupOnMouseClick->isChecked());
    cfg.writeEntry("zoomSize", ui->popupSize->value());

    cfg.writeEntry("useCustomColorSpace", ui->useCustomColorSpace->isChecked());
    const KoColorSpace* colorSpace = ui->colorSpace->currentColorSpace();
    if(colorSpace) {
        cfg.writeEntry("customColorSpaceModel", colorSpace->colorModelId().id());
        cfg.writeEntry("customColorSpaceDepthID", colorSpace->colorDepthId().id());
        cfg.writeEntry("customColorSpaceProfile", colorSpace->profile()->name());
    }

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

    cfg.writeEntry("minimalShadeSelectorAsGradient", ui->minimalShadeSelectorAsGradient->isChecked());
    cfg.writeEntry("minimalShadeSelectorPatchCount", ui->minimalShadeSelectorPatchesPerLine->value());
    cfg.writeEntry("minimalShadeSelectorLineConfig",  ui->minimalShadeSelectorLineSettings->toString());
    cfg.writeEntry("minimalShadeSelectorLineHeight", ui->minimalShadeSelectorLineHeight->value());

    //color selector
    KisColorSelectorComboBox* cstw = dynamic_cast<KisColorSelectorComboBox*>(ui->colorSelectorConfiguration);
    cfg.writeEntry("colorSelectorConfiguration", cstw->configuration().toString());

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

    if(cfg.readEntry("useCustomColorSpace", false))
        ui->useCustomColorSpace->setChecked(true);
    else
        ui->useImageColorSpace->setChecked(true);

    ui->colorSpace->setCurrentColorModel(KoID(cfg.readEntry("customColorSpaceModel", "RGBA")));
    ui->colorSpace->setCurrentColorDepth(KoID(cfg.readEntry("customColorSpaceDepthID", "U8")));
    ui->colorSpace->setCurrentProfile(cfg.readEntry("customColorSpaceProfile", KoColorSpaceRegistry::instance()->rgb8()->profile()->name()));

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

    bool asGradient = cfg.readEntry("minimalShadeSelectorAsGradient", true);
    if(asGradient) ui->minimalShadeSelectorAsGradient->setChecked(true);
    else ui->minimalShadeSelectorAsColorPatches->setChecked(true);

    ui->minimalShadeSelectorPatchesPerLine->setValue(cfg.readEntry("minimalShadeSelectorPatchCount", 10));
    ui->minimalShadeSelectorLineSettings->fromString(cfg.readEntry("minimalShadeSelectorLineConfig", "0|0.2|0|0|0|0|0;1|0|1|1|0|0|0;2|0|-1|1|0|0|0;"));
    ui->minimalShadeSelectorLineHeight->setValue(cfg.readEntry("minimalShadeSelectorLineHeight", 10));

    //color selector
    KisColorSelectorComboBox* cstw = dynamic_cast<KisColorSelectorComboBox*>(ui->colorSelectorConfiguration);
    cstw->setConfiguration(KisColorSelector::Configuration::fromString(cfg.readEntry("colorSelectorConfiguration", "3|0|5|0"))); // triangle selector
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
    KisColorSelectorComboBox* cstw = dynamic_cast<KisColorSelectorComboBox*>(ui->colorSelectorConfiguration);
    cstw->setConfiguration(KisColorSelector::Configuration("3|0|5|0")); // triangle selector
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

