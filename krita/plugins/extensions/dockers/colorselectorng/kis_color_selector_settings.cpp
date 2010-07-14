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

#include <KConfigGroup>
#include "kis_color_selector_type_widget.h"
#include "kis_color_selector.h"

#include <KDebug>

KisColorSelectorSettings::KisColorSelectorSettings(KisCanvas2* canvas, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KisColorSelectorSettings)
{
    ui->setupUi(this);
    ui->lbl_lastUsedNumRows->hide();
    ui->lastUsedColorsNumRows->hide();

    ui->lbl_commonColorsNumCols->hide();
    ui->commonColorsNumCols->hide();
    resize(minimumSize());

    ui->colorSelectorConfiguration->setCanvas(canvas);

    connect(this, SIGNAL(accepted()), this, SLOT(settingsAccepted()));

    readSettings();
}

KisColorSelectorSettings::~KisColorSelectorSettings()
{
    delete ui;
}

void KisColorSelectorSettings::settingsAccepted()
{
    //write cfg
    KConfigGroup cfg = KGlobal::config()->group("extendedColorSelector");

    //general
    cfg.writeEntry("shadeSelectorHideable", ui->shadeSelectorHideable->isChecked());
    cfg.writeEntry("allowHorizontalLayout", ui->allowHorizontalLayout->isChecked());
    cfg.writeEntry("popupOnMouseOver", ui->popupOnMouseOver->isChecked());
    cfg.writeEntry("popupOnMouseClick", ui->popupOnMouseClick->isChecked());
    cfg.writeEntry("zoomSize", ui->popupSize->value());

    //color patches
    cfg.writeEntry("showLastUsedColors", ui->lastUsedColorsShow->isChecked());
    cfg.writeEntry("lastUsedColorsAlignment", ui->lastUsedColorsAlignVertical->isChecked());
    cfg.writeEntry("lastUsedColorsScrolling", ui->lastUsedColorsAllowScrolling->isChecked());
    cfg.writeEntry("lastUsedColorsNumCols", ui->lastUsedColorsNumCols->value());
    cfg.writeEntry("lastUsedColorsNumRows", ui->lastUsedColorsNumRows->value());
    cfg.writeEntry("lastUsedColorsCount", ui->lastUsedColorsPatchCount->value());
    cfg.writeEntry("lastUsedColorsWidth", ui->lastUsedColorsWidth->value());
    cfg.writeEntry("lastUsedColorsHeight", ui->lastUsedColorsHeight->value());

    cfg.writeEntry("showCommonColors", ui->commonColorsShow->isChecked());
    cfg.writeEntry("commonColorsAlignment", ui->commonColorsAlignVertical->isChecked());
    cfg.writeEntry("commonColorsScrolling", ui->commonColorsAllowScrolling->isChecked());
    cfg.writeEntry("commonColorsNumCols", ui->commonColorsNumCols->value());
    cfg.writeEntry("commonColorsNumRows", ui->commonColorsNumRows->value());
    cfg.writeEntry("commonColorsCount", ui->commonColorsPatchCount->value());
    cfg.writeEntry("commonColorsWidth", ui->commonColorsWidth->value());
    cfg.writeEntry("commonColorsHeight", ui->commonColorsHeight->value());
    cfg.writeEntry("commonColorsAutoUpdate", ui->commonColorsAutoUpdate->isChecked());

    //shade selector
    cfg.writeEntry("shadeSelectorType", ui->shadeSelectorType->currentIndex());

    //color selector
    KisColorSelectorTypeWidget* cstw = dynamic_cast<KisColorSelectorTypeWidget*>(ui->colorSelectorConfiguration);
    cfg.writeEntry("colorSelectorConfiguration", cstw->configuration().toString());
}

void KisColorSelectorSettings::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void KisColorSelectorSettings::readSettings()
{
    //read cfg
    KConfigGroup cfg = KGlobal::config()->group("extendedColorSelector");

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
    ui->lastUsedColorsShow->setChecked(cfg.readEntry("showLastUsedColors", true));
    a = cfg.readEntry("lastUsedColorsAlignment", false);
    ui->lastUsedColorsAlignVertical->setChecked(a);
    ui->lastUsedColorsAlignHorizontal->setChecked(!a);
    ui->lastUsedColorsAllowScrolling->setChecked(cfg.readEntry("lastUsedColorsScrolling", true));
    ui->lastUsedColorsNumCols->setValue(cfg.readEntry("lastUsedColorsNumCols", 1));
    ui->lastUsedColorsNumRows->setValue(cfg.readEntry("lastUsedColorsNumRows", 1));
    ui->lastUsedColorsPatchCount->setValue(cfg.readEntry("lastUsedColorsCount", 15));
    ui->lastUsedColorsWidth->setValue(cfg.readEntry("lastUsedColorsWidth", 20));
    ui->lastUsedColorsHeight->setValue(cfg.readEntry("lastUsedColorsHeight", 20));

    ui->commonColorsShow->setChecked(cfg.readEntry("showCommonColors", true));
    a = cfg.readEntry("commonColorsAlignment", true);
    ui->commonColorsAlignVertical->setChecked(a);
    ui->commonColorsAlignHorizontal->setChecked(!a);
    ui->commonColorsAllowScrolling->setChecked(cfg.readEntry("commonColorsScrolling", false));
    ui->commonColorsNumCols->setValue(cfg.readEntry("commonColorsNumCols", 1));
    ui->commonColorsNumRows->setValue(cfg.readEntry("commonColorsNumRows", 2));
    ui->commonColorsPatchCount->setValue(cfg.readEntry("commonColorsCount", 20));
    ui->commonColorsWidth->setValue(cfg.readEntry("commonColorsWidth", 20));
    ui->commonColorsHeight->setValue(cfg.readEntry("commonColorsHeight", 20));
    ui->commonColorsAutoUpdate->setChecked(cfg.readEntry("commonColorsAutoUpdate", false));

    //shade selector
    ui->shadeSelectorType->setCurrentIndex(cfg.readEntry("shadeSelectorType", 0));

    //color selector
    KisColorSelectorTypeWidget* cstw = dynamic_cast<KisColorSelectorTypeWidget*>(ui->colorSelectorConfiguration);
    cstw->setConfiguration(KisColorSelector::Configuration::fromString(cfg.readEntry("colorSelectorConfiguration", KisColorSelector::Configuration().toString())));
}
