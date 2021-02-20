/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_edit_profiles_dialog.h"

#include <QPushButton>
#include <QStringListModel>
#include <KLocalizedString>
#include <QMessageBox>

#include "kis_icon_utils.h"
#include "input/kis_input_profile_manager.h"
#include "kis_input_profile_model.h"

#include "ui_kis_edit_profiles_dialog.h"

class KisEditProfilesDialog::Private
{
public:
    Private() { }

    Ui::KisEditProfilesDialog *ui;
    KisInputProfileModel *profileModel;
};

KisEditProfilesDialog::KisEditProfilesDialog(QWidget *parent, Qt::WindowFlags flags)
    : KoDialog(parent, flags), d(new Private())
{
    QWidget *mainWidget = new QWidget(this);
    d->ui = new Ui::KisEditProfilesDialog();
    d->ui->setupUi(mainWidget);
    setMainWidget(mainWidget);

    d->profileModel = new KisInputProfileModel(this);
    d->ui->profileList->setModel(d->profileModel);

    d->ui->removeButton->setIcon(KisIconUtils::loadIcon("edit-delete"));
    d->ui->duplicateButton->setIcon(KisIconUtils::loadIcon("edit-copy"));
    d->ui->renameButton->setIcon(KisIconUtils::loadIcon("document-edit"));
    d->ui->resetButton->setIcon(KisIconUtils::loadIcon("view-refresh"));

    connect(d->ui->removeButton, SIGNAL(clicked(bool)), SLOT(removeButtonClicked()));
    connect(d->ui->duplicateButton, SIGNAL(clicked(bool)), SLOT(duplicateButtonClicked()));
    connect(d->ui->renameButton, SIGNAL(clicked(bool)), SLOT(renameButtonClicked()));
    connect(d->ui->resetButton, SIGNAL(clicked(bool)), SLOT(resetButtonClicked()));

    d->ui->removeButton->setEnabled(d->profileModel->rowCount() > 1);

    setButtons(Close | Default);
    setWindowTitle(i18n("Edit Profiles"));
}

KisEditProfilesDialog::~KisEditProfilesDialog()
{
    delete d;
}

void KisEditProfilesDialog::removeButtonClicked()
{
    KisInputProfileManager::instance()->removeProfile(d->profileModel->profileName(d->ui->profileList->currentIndex()));
    d->ui->removeButton->setEnabled(d->profileModel->rowCount() > 1);
}

void KisEditProfilesDialog::duplicateButtonClicked()
{
    QString currentName = d->profileModel->profileName(d->ui->profileList->currentIndex());
    QString newName = i18n("Copy of %1", currentName);
    KisInputProfileManager::instance()->duplicateProfile(currentName, newName);
    d->ui->removeButton->setEnabled(d->profileModel->rowCount() > 1);
}

void KisEditProfilesDialog::renameButtonClicked()
{
    d->ui->profileList->edit(d->ui->profileList->currentIndex());
}

void KisEditProfilesDialog::resetButtonClicked()
{
    if(QMessageBox::question(this,
                             i18nc("@title:window", "Reset All Profiles"),
                             i18n("You will lose all changes to any input profiles. Do you wish to continue?"),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::Yes) == QMessageBox::Yes) {
        KisInputProfileManager::instance()->resetAll();
    }
}
