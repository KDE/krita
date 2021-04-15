/*
 *  SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "DlgHeifImport.h"
#include "ui_DlgHeifImport.h"

DlgHeifImport::DlgHeifImport(bool apply, float gamma, float brightness, QWidget *parent) :
    KoDialog(parent),
    ui(new Ui::DlgHeifImport)
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

bool DlgHeifImport::applyOOTF()
{
    return ui->chkApplyOOTF->isChecked();
}

float DlgHeifImport::gamma()
{
    return ui->spnGamma->value();
}

float DlgHeifImport::nominalPeakBrightness()
{
    return ui->spnNits->value();
}

void DlgHeifImport::toggleHLGOptions(bool toggle)
{
    ui->spnNits->setEnabled(toggle);
    ui->spnGamma->setEnabled(toggle);
}

#include <DlgHeifImport.moc>
