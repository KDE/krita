/*
 *  Copyright (c) 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "recorder_profile_settings.h"
#include "recorder_export_config.h"
#include "ui_recorder_profile_settings.h"

#include <klocalizedstring.h>
#include <kis_icon_utils.h>

RecorderProfileSettings::RecorderProfileSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RecorderProfileSettings)
{
    ui->setupUi(this);

    ui->buttonPresetRevert->setIcon(KisIconUtils::loadIcon("edit-undo"));
}

RecorderProfileSettings::~RecorderProfileSettings()
{
    delete ui;
}

bool RecorderProfileSettings::editProfile(RecorderProfile *profile, const RecorderProfile &defaultProfile)
{
    fillProfile(*profile);

    disconnect(ui->buttonPresetRevert);
    connect(ui->buttonPresetRevert, &QPushButton::clicked, [&] { fillProfile(defaultProfile); });

    if (exec() != QDialog::Accepted)
        return false;

    profile->name = ui->editProfileName->text();
    profile->extension = ui->editFileExtension->text();
    profile->arguments = ui->editFfmpegArguments->toPlainText();

    return true;
}

void RecorderProfileSettings::onInputChanged()
{
    const QString &name = ui->editProfileName->text();
    const QString &extension = ui->editFileExtension->text();
    const QString &arguments = ui->editFfmpegArguments->toPlainText();

    bool isValid = (!name.isEmpty()) && (!extension.isEmpty()) && (!arguments.isEmpty());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
}

void RecorderProfileSettings::fillProfile(const RecorderProfile &profile)
{
    ui->editProfileName->setText(profile.name);
    ui->editFileExtension->setText(profile.extension);
    ui->editFfmpegArguments->setPlainText(profile.arguments);
}
