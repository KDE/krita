/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_profile_settings.h"
#include "recorder_export_config.h"
#include "ui_recorder_profile_settings.h"

#include <klocalizedstring.h>
#include <kstandardguiitem.h>
#include <kis_icon_utils.h>

namespace
{
enum ArgumentsPageIndex
{
    PageEdit,
    PagePreview
};
}


RecorderProfileSettings::RecorderProfileSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RecorderProfileSettings)
{
    ui->setupUi(this);

    KGuiItem::assign(ui->buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(ui->buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());

    ui->buttonPresetRevert->setIcon(KisIconUtils::loadIcon("edit-undo"));
    ui->stackedWidget->setCurrentIndex(ArgumentsPageIndex::PageEdit);

    connect(ui->labelSupportedVariables, SIGNAL(linkActivated(QString)), this, SLOT(onLinkActivated(QString)));
    connect(ui->checkPreview, SIGNAL(toggled(bool)), this, SLOT(onPreviewToggled(bool)));
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

void RecorderProfileSettings::setPreview(const QString &preview)
{
    ui->editPreview->setPlainText(preview);
}

void RecorderProfileSettings::onInputChanged()
{
    const QString &name = ui->editProfileName->text();
    const QString &extension = ui->editFileExtension->text();
    const QString &arguments = ui->editFfmpegArguments->toPlainText();

    bool isValid = (!name.isEmpty()) && (!extension.isEmpty()) && (!arguments.isEmpty());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
}

void RecorderProfileSettings::onLinkActivated(const QString &link)
{
    ui->editFfmpegArguments->insertPlainText(link);
    ui->editFfmpegArguments->setFocus();
}

void RecorderProfileSettings::onPreviewToggled(bool checked)
{
    if (checked)
        emit requestPreview(ui->editFfmpegArguments->toPlainText());

    ui->stackedWidget->setCurrentIndex(checked ? ArgumentsPageIndex::PagePreview : ArgumentsPageIndex::PageEdit);
}

void RecorderProfileSettings::fillProfile(const RecorderProfile &profile)
{
    ui->editProfileName->setText(profile.name);
    ui->editFileExtension->setText(profile.extension);
    ui->editFfmpegArguments->setPlainText(profile.arguments);
}
