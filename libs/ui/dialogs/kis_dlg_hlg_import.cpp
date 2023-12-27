/*
 *  SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_dlg_hlg_import.h"
#include "ui_kis_dlg_hlg_import.h"

KisDlgHLGImport::KisDlgHLGImport(bool apply, float gamma, float brightness, QWidget *parent)
    : KoDialog(parent)
    , ui(new Ui::DlgHeifImport)
{
    QWidget *page = new QWidget(this);
    ui->setupUi(page);
    setMainWidget(page);
    ui->chkApplyOOTF->setChecked(apply);
    ui->spnGamma->setValue(gamma);
    ui->spnNits->setValue(brightness);

    toggleHLGOptions(applyOOTF());

    connect(ui->chkApplyOOTF, SIGNAL(toggled(bool)), this, SLOT(toggleHLGOptions(bool)));
}

bool KisDlgHLGImport::applyOOTF()
{
    return ui->chkApplyOOTF->isChecked();
}

float KisDlgHLGImport::gamma()
{
    return ui->spnGamma->value();
}

float KisDlgHLGImport::nominalPeakBrightness()
{
    return ui->spnNits->value();
}

void KisDlgHLGImport::toggleHLGOptions(bool toggle)
{
    ui->spnNits->setEnabled(toggle);
    ui->spnGamma->setEnabled(toggle);
}
