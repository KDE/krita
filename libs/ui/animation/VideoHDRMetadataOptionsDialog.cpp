/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "VideoHDRMetadataOptionsDialog.h"
#include "ui_VideoHDRMetadataOptionsDialog.h"

#include "KisHDRMetadataOptions.h"

VideoHDRMetadataOptionsDialog::VideoHDRMetadataOptionsDialog(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::VideoHDRMetadataOptionsDialog)
{
    ui->setupUi(this);

    connect(ui->btnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->btnBox, SIGNAL(rejected()), this, SLOT(reject()));

    ui->cmbMasterDisplay->addItem(i18n("Rec. 2100 PQ"), "p2100-pq");
    ui->cmbMasterDisplay->addItem(i18n("DCI-P3 D65"), "dci-p3-d65");
    ui->cmbMasterDisplay->addItem(i18n("Custom"), "custom");

    connect(ui->cmbMasterDisplay, SIGNAL(currentIndexChanged(int)), SLOT(slotPredefinedDisplayIdChanged()));
}

VideoHDRMetadataOptionsDialog::~VideoHDRMetadataOptionsDialog()
{
    delete ui;
}

void VideoHDRMetadataOptionsDialog::setHDRMetadataOptions(const KisHDRMetadataOptions &options)
{
    ui->dblRedX->setValue(options.redX);
    ui->dblRedY->setValue(options.redY);

    ui->dblGreenX->setValue(options.greenX);
    ui->dblGreenY->setValue(options.greenY);

    ui->dblBlueX->setValue(options.blueX);
    ui->dblBlueY->setValue(options.blueY);

    ui->dblWhiteX->setValue(options.whiteX);
    ui->dblWhiteY->setValue(options.whiteY);

    ui->dblMinLuminance->setValue(options.minLuminance);
    ui->dblMaxLuminance->setValue(options.maxLuminance);

    ui->intMaxCLL->setValue(options.maxCLL);
    ui->intMaxFALL->setValue(options.maxFALL);

    int index = ui->cmbMasterDisplay->findData(options.predefinedMasterDisplayId);
    if (index < 0) {
        index = ui->cmbMasterDisplay->findData("custom");
    }
    ui->cmbMasterDisplay->setCurrentIndex(index);

    slotPredefinedDisplayIdChanged();
}

KisHDRMetadataOptions VideoHDRMetadataOptionsDialog::hdrMetadataOptions() const
{
    KisHDRMetadataOptions options;

    ui->dblRedX->setValue(options.redX);
    options.redY = ui->dblRedY->value();

    options.greenX = ui->dblGreenX->value();
    options.greenY = ui->dblGreenY->value();

    options.blueX = ui->dblBlueX->value();
    options.blueY = ui->dblBlueY->value();

    options.whiteX = ui->dblWhiteX->value();
    options.whiteY = ui->dblWhiteY->value();

    options.minLuminance = ui->dblMinLuminance->value();
    options.maxLuminance = ui->dblMaxLuminance->value();

    options.maxCLL = ui->intMaxCLL->value();
    options.maxFALL = ui->intMaxFALL->value();

    options.predefinedMasterDisplayId = ui->cmbMasterDisplay->currentData().toString();

    return options;
}

void VideoHDRMetadataOptionsDialog::slotPredefinedDisplayIdChanged()
{
    const QString displayId = ui->cmbMasterDisplay->currentData().toString();

    if (displayId == "p2100-pq") {
        ui->grpCustomDisplay->setEnabled(false);

        ui->dblRedX->setValue(0.708);
        ui->dblRedY->setValue(0.292);

        ui->dblGreenX->setValue(0.170);
        ui->dblGreenY->setValue(0.797);

        ui->dblBlueX->setValue(0.131);
        ui->dblBlueY->setValue(0.046);

        ui->dblWhiteX->setValue(0.3127);
        ui->dblWhiteY->setValue(0.3290);

        ui->dblMinLuminance->setValue(0.005);
        ui->dblMaxLuminance->setValue(1000);

    } else if (displayId == "dci-p3-d65") {
        ui->grpCustomDisplay->setEnabled(false);

        ui->dblRedX->setValue(0.680);
        ui->dblRedY->setValue(0.320);

        ui->dblGreenX->setValue(0.265);
        ui->dblGreenY->setValue(0.690);

        ui->dblBlueX->setValue(0.150);
        ui->dblBlueY->setValue(0.060);

        ui->dblWhiteX->setValue(0.3127);
        ui->dblWhiteY->setValue(0.3290);

        ui->dblMinLuminance->setValue(0.005);
        ui->dblMaxLuminance->setValue(1000);

    } else {
        ui->grpCustomDisplay->setEnabled(true);
    }

}
